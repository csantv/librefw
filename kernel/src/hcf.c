#include "hcf.h"
#include "hcf_internal.h"
#include "nl.h"
#include "nl_ops.h"

#include <linux/math.h>
#include <linux/mutex.h>
#include <linux/unaligned.h>
#include <net/genetlink.h>

static struct hcf_state *state = NULL;
static DEFINE_MUTEX(lock);

int hcf_init_state(void)
{
    int ret = -ENOMEM;
    state = kzalloc(sizeof(struct hcf_state), GFP_KERNEL);
    if (state == NULL) {
        return -ENOMEM;
    }
    struct hcf_state *st = state;

    st->mem = kmem_cache_create("lfw_hcf_cache", sizeof(struct hcf_node), 0, SLAB_HWCACHE_ALIGN, NULL);
    if (st->mem == NULL) {
        goto err_free_state;
    }

    st->workqueue = alloc_workqueue("lfw_hcf_wq", WQ_UNBOUND | WQ_HIGHPRI, 0);
    if (!st->workqueue) {
        goto err_kmem_free;
    }

    struct hcf_node *new_tree = hcf_create_node();
    if (!new_tree) {
        goto err_destroy_wq;
    }

    rcu_assign_pointer(st->tree, new_tree);
    return 0;

err_destroy_wq:
    destroy_workqueue(st->workqueue);
    st->workqueue = NULL;
err_kmem_free:
    kmem_cache_destroy(st->mem);
    st->mem = NULL;
err_free_state:
    kfree(st);
    state = NULL;
    return ret;
}

void hcf_free_state(void)
{
    if (state->workqueue) {
        destroy_workqueue(state->workqueue);
    }

    hcf_swap_tree(NULL);
    kmem_cache_destroy(state->mem);
    kfree(state);
}

void hcf_swap_tree(struct hcf_node *new_tree)
{
    mutex_lock(&lock);
    struct hcf_node *old_root = rcu_dereference_protected(state->tree, lockdep_is_held(&lock));
    rcu_assign_pointer(state->tree, new_tree);
    mutex_unlock(&lock);

    if (!old_root) {
        return;
    }

    synchronize_rcu();
    hcf_free_node(old_root);
}

void hcf_free_node(struct hcf_node *node)
{
    if (!node) {
        return;
    }

    hcf_free_node(node->child[0]);
    hcf_free_node(node->child[1]);

    kmem_cache_free(state->mem, node);

    // call_rcu(&node->rcu, hcf_free_node_rcu);
}

struct hcf_node *hcf_create_node(void)
{
    struct hcf_node *node = kmem_cache_zalloc(state->mem, GFP_KERNEL);
    if (node == NULL) {
        pr_err("librefw: could not allocate memory for hc node\n");
        return NULL;
    }
    node->child[0] = NULL;
    node->child[1] = NULL;
    node->hc = 0;
    node->ttl = 0;
    return node;
}

struct hcf_node *hcf_clone_node(struct hcf_node *old)
{
    struct hcf_node *new_node = kmem_cache_zalloc(state->mem, GFP_KERNEL);
    if (new_node) {
        *new_node = *old;
    }
    return new_node;
}

void hcf_register_ip_work(struct work_struct *work)
{
    struct hcf_node *nodes_to_free[33];
    int free_count = 0, nodes_created = 0;

    struct hcf_node *new_root, *runner, *new_runner;
    struct hcf_add_node_task *task = container_of(work, struct hcf_add_node_task, real_work);
    mutex_lock(&lock);

    // clone the whole branch until you get to a non rcu portion, then create remaining branch
    runner = rcu_dereference_protected(state->tree, lockdep_is_held(&lock));
    if (!runner) {
        mutex_unlock(&lock);
        goto end;
    }

    new_root = hcf_clone_node(runner);
    new_runner = new_root;
    nodes_to_free[free_count++] = runner;

    // clone rcu protected path
    int remaining_bits = 31;
    for (; remaining_bits >= 8; remaining_bits--) {
        int bit = (task->source_ip >> remaining_bits) & 1;
        struct hcf_node *old_child = rcu_dereference_protected(runner->child[bit], lockdep_is_held(&lock));
        if (!old_child) {
            break;
        }
        struct hcf_node *new_child = hcf_clone_node(old_child);
        new_runner->child[bit] = new_child;
        nodes_to_free[free_count++] = old_child;

        runner = old_child;
        new_runner = new_child;
    }

    // create remaining leaf, no rcu here
    for (; remaining_bits >= 8; remaining_bits--) {
        int bit = (task->source_ip >> remaining_bits) & 1;
        new_runner->child[bit] = hcf_create_node();
        new_runner = new_runner->child[bit];
        nodes_created++;
    }

    // asign metadata
    new_runner->ttl = task->ttl;
    new_runner->hc = hcf_get_initial_ttl(task->ttl) - task->ttl;

    rcu_assign_pointer(state->tree, new_root);

    mutex_unlock(&lock);
    if (nodes_created > 0) {
        hcf_log_new_ip(task->source_ip, new_runner->ttl, new_runner->hc);
        u32 net_ip = htonl(task->source_ip);
        pr_info_ratelimited("librefw: [hcf] adding new node for ip %pI4 with hc %d, %d nodes created\n", &net_ip,
                            new_runner->hc, nodes_created);
    }

    for (int i = 0; i < free_count; i++) {
        call_rcu(&nodes_to_free[i]->rcu, hcf_free_node_rcu);
    }

end:
    kfree(task);
}

void hcf_free_node_rcu(struct rcu_head *rp)
{
    struct hcf_node *node = container_of(rp, struct hcf_node, rcu);
    kmem_cache_free(state->mem, node);
}

int hcf_get_initial_ttl(u8 ttl)
{
    if (ttl <= 64)
        return 64;
    if (ttl <= 128)
        return 128;
    return 255;
}

int hcf_lookup_tree(u32 source_ip, u8 ttl)
{
    int result = -1;
    rcu_read_lock();
    struct hcf_node *runner = rcu_dereference(state->tree);
    if (!runner) {
        goto end;
    }

    for (int i = 31; i >= 8; i--) {
        int bit = (source_ip >> i) & 1;
        runner = rcu_dereference(runner->child[bit]);
        if (!runner) {
            goto end;
        }
    }

    u8 initial_ttl = hcf_get_initial_ttl(ttl);
    int current_hc = initial_ttl - ttl;
    int stored_hc = (int)READ_ONCE(runner->hc);

    if (abs(current_hc - stored_hc) <= 2) {
        result = 1;
    } else {
        result = 0;
    }

end:
    rcu_read_unlock();
    return result;
}

void hcf_register_ip(u32 source_ip, u8 ttl)
{
    struct hcf_add_node_task *task = kzalloc(sizeof(struct hcf_add_node_task), GFP_ATOMIC);
    if (task == NULL) {
        return;
    }

    task->source_ip = source_ip;
    task->ttl = ttl;
    INIT_WORK(&task->real_work, hcf_register_ip_work);

    // task was already queued (rare)
    if (!queue_work(state->workqueue, &task->real_work)) {
        kfree(task);
    }
}

int hcf_log_new_ip(u32 source_ip, u8 ttl, u8 hc)
{
    struct hcf_ip_ctx ctx = {source_ip, ttl, hc};
    return lfw_make_multicast_msg(LFW_NL_GROUP_HCF, LFW_NL_CMD_HCF, &ctx, hcf_build_log_msg);
}

int hcf_build_log_msg(struct sk_buff *skb, void *data)
{
    struct hcf_ip_ctx *ctx = data;

    if (nla_put_u32(skb, LFW_NLA_HCF_IP, ctx->source_ip) || nla_put_u8(skb, LFW_NLA_HCF_TTL, ctx->ttl) ||
        nla_put_u8(skb, LFW_NLA_HCF_HC, ctx->hc)) {
        pr_err("librefw: could not build hcf log message\n");
        return -EMSGSIZE;
    }

    return 0;
}

int hcf_register_ip_history(struct sk_buff *skb, struct genl_info *info)
{
    struct hcf_node *new_tree = hcf_create_node();
    if (unlikely(!new_tree)) {
        pr_err("librefw: could not allocate memory for new hcf tree\n");
        return -ENOMEM;
    }

    int new_nodes = 0, new_ips = 0;
    struct nlattr *pos = NULL;
    int rem = 0;
    nla_for_each_attr_type(pos, LFW_NLA_HCF_HISTORY, nlmsg_attrdata(info->nlhdr, GENL_HDRLEN),
                           nlmsg_attrlen(info->nlhdr, GENL_HDRLEN), rem)
    {
        int nested_rem = 0;
        struct nlattr *nested_pos = NULL;

        struct hcf_node *runner = new_tree;
        __be32 ip_addr_be = 0;
        u32 ip_addr = 0;
        u8 ttl = 0, hc = 0;
        nla_for_each_nested(nested_pos, pos, nested_rem)
        {
            switch (nla_type(nested_pos)) {
                case LFW_NLA_HCF_HISTORY_IP: {
                    int data_len = nla_len(nested_pos);

                    if (data_len == 4) {
                        ip_addr = nla_get_in_addr(nested_pos); // nested_pos comes as host byte order, this is wrong
                        // must start saving and sending IPs as network byte
                        // order
                        ip_addr_be = cpu_to_be32(ip_addr);
                    }
                    break;
                }
                case LFW_NLA_HCF_HISTORY_TTL:
                    ttl = nla_get_u8(nested_pos);
                    break;
                case LFW_NLA_HCF_HISTORY_HC:
                    hc = nla_get_u8(nested_pos);
                    break;
                default:
                    pr_warn("librefw: got unexpected value in hcf history\n");
            }
        }

        for (int i = 31; i >= 8; i--) {
            int bit = (ip_addr >> i) & 1;
            if (!runner->child[bit]) {
                runner->child[bit] = hcf_create_node();
                new_nodes++;
            }
            runner = runner->child[bit];
        }
        runner->ttl = ttl;
        runner->hc = hc;

        new_ips++;
        pr_info("librefw: inserted ip %pI4, ttl=%d, hc=%d\n", &ip_addr_be, ttl, hc);
    }

    hcf_swap_tree(new_tree);
    pr_info("librefw: inserted hcf ip history, new ips=%d, new nodes=%d\n", new_ips, new_nodes);

    return 0;
}
