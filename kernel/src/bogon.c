#include "bogon.h"
#include "nl_ops.h"
#include "utils.h"
#include "log.h"

#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/inet.h>
#include <linux/kstrtox.h>
#include <linux/unaligned.h>
#include <net/genetlink.h>

static struct lfw_bg_state *state = NULL;
static DEFINE_SPINLOCK(lock);

int lfw_init_bg_state(void)
{
    int ret;
    state = kzalloc(sizeof(struct lfw_bg_state), GFP_KERNEL);
    if (unlikely(state == NULL)) {
        return -ENOMEM;
    }
    struct lfw_bg_state *st = state;

    st->mem = kmem_cache_create("lfw_bogon_cache", sizeof(struct lfw_bg_node), 0, SLAB_HWCACHE_ALIGN, NULL);
    if (st->mem == NULL) {
        ret = -ENOMEM;
        goto err_free_state;
    }

    struct lfw_bg_tree *tree = kzalloc(sizeof(struct lfw_bg_tree), GFP_KERNEL);
    if (tree == NULL) {
        ret = -ENOMEM;
        goto err_kmem_free;
    }

    rcu_assign_pointer(st->tree, tree); // NOLINT
    return 0;

err_kmem_free:
    kmem_cache_destroy(st->mem);
    st->mem = NULL;
err_free_state:
    kfree(st);
    state = NULL;
    return ret;
}

void lfw_free_bg_state(void)
{
    if (unlikely(state == NULL)) {
        return;
    }

    struct lfw_bg_state *st = state;

    call_rcu(&st->tree->rcu, lfw_free_bg_tree);
    synchronize_rcu();

    kmem_cache_destroy(st->mem);
    kfree(st);
}

void lfw_free_bg_tree(struct rcu_head *rp)
{
    struct lfw_bg_tree *tree = container_of(rp, struct lfw_bg_tree, rcu);
    lfw_free_bg_node(tree->root);
    kfree(tree);
}

struct lfw_bg_node* lfw_create_node(void)
{
    struct lfw_bg_node* node = kmem_cache_zalloc(state->mem, GFP_KERNEL);
    if (node == NULL) {
        return NULL;
    }
    node->child[0] = NULL;
    node->child[1] = NULL;
    node->is_bogon = 0;
    return node;
}

void lfw_free_bg_node(struct lfw_bg_node *node)
{
    if (node == NULL) {
        return;
    }

    lfw_free_bg_node(node->child[0]);
    lfw_free_bg_node(node->child[1]);

    kmem_cache_free(state->mem, node);
}

void lfw_load_bg_tree(struct lfw_ip_prefix *prefixes, u32 len)
{
    struct lfw_bg_tree *tree = kzalloc(sizeof(struct lfw_bg_tree), GFP_KERNEL);
    if (unlikely(tree == NULL)) {
        pr_err("librefw: could not allocate memory for new bogon tree\n");
        return;
    }
    tree->root = lfw_create_node();
    if (unlikely(tree->root == NULL)) {
        pr_err("librefw: could not allocate memory for new bogon node\n");
        return;
    }

    for (int i = 0; i < len; ++i) {
        u32 ip_be = prefixes[i].ip_prefix;
        u32 ip = get_unaligned_be32(&ip_be);
        size_t num_nodes = 0;
        struct lfw_bg_node *runner = tree->root;

        char ip_path[32] = {};
        for (int j = 0; j < prefixes[i].ip_prefix_len; j++) {
            int bit = in4_get_bit(ip, j);
            ip_path[j] = bit + '0';
            if (runner->child[bit] == NULL) {
                runner->child[bit] = lfw_create_node();

                if (unlikely(tree->root == NULL)) {
                    pr_err("librefw: could not allocate memory for new bogon node\n");
                    return;
                }
                num_nodes++;
            }
            runner = runner->child[bit];
        }
        runner->is_bogon = 1;
        lfw_log(LOGLEVEL_INFO, "done parsing ip %pI4, used path %s, %ld nodes created", &ip_be, ip_path, num_nodes);
    }

    pr_info("librefw: replacing old tree with new tree\n");
    spin_lock(&lock);
    struct lfw_bg_tree *old_tree = rcu_dereference_protected(state->tree, lockdep_is_held(&lock));
    rcu_assign_pointer(state->tree, tree); // NOLINT
    spin_unlock(&lock);

    call_rcu(&old_tree->rcu, lfw_free_bg_tree);
}

int lfw_lookup_bg_tree(u32 ip)
{
    rcu_read_lock();
    struct lfw_bg_tree *tree = rcu_dereference(state->tree); // NOLINT
    int result = 0;

    struct lfw_bg_node *runner = tree->root;
    if (runner == NULL) {
        result = -1;
        goto end;
    }

    for (int i = 0; i < 32; i++) {
        int bit = in4_get_bit(ip, i);

        if (runner->is_bogon) {
            result = 1;
            goto end;
        }

        runner = runner->child[bit];

        if (runner == NULL) {
            result = -2;
            goto end;
        }
    }

    if (runner != NULL && runner->is_bogon) {
        result = 1;
    }

end:
    rcu_read_unlock();
    return result;
}

int lfw_bogon_set(struct sk_buff *skb, struct genl_info *info)
{
    u32 num_prefix = nla_get_u32_default(info->attrs[LFW_NLA_NUM_IP_PREFIX], 0);
    if (!num_prefix) {
        pr_warn("librefw: did not receive number of prefixes\n");
        return -EINVAL;
    }

    pr_info("librefw: inserting %d prefixes into bogon filter list\n", num_prefix);
    struct lfw_ip_prefix *prefix_buf = vzalloc(sizeof(struct lfw_ip_prefix) * num_prefix);
    if (prefix_buf == NULL) {
        pr_warn("librefw: could not allocate buffer for ip prefixes\n");
        return -ENOMEM;
    }
    struct lfw_ip_prefix *runner = prefix_buf;

    struct nlattr *pos = NULL;
    int rem = 0;
    nla_for_each_attr_type(pos, LFW_NLA_IP_PREFIX, nlmsg_attrdata(info->nlhdr, GENL_HDRLEN),
                           nlmsg_attrlen(info->nlhdr, GENL_HDRLEN), rem)
    {
        int nested_rem = 0;
        struct nlattr *nested_pos = NULL;
        nla_for_each_nested(nested_pos, pos, nested_rem)
        {
            switch (nla_type(nested_pos)) {
                case LFW_NLA_N_IP_ADDR:
                    runner->ip_prefix = nla_get_u32(nested_pos);
                    break;
                case LFW_NLA_N_IP_PREFIX_LEN:
                    runner->ip_prefix_len = nla_get_u8(nested_pos);
                    break;
                default:
                    pr_warn("librefw: got unexpected value in bogon message\n");
            }
        }
        runner++;
    }

    lfw_load_bg_tree(prefix_buf, num_prefix);
    vfree(prefix_buf);

    lfw_log(LOGLEVEL_INFO, "done inserting %d prefixes into bogon filter list", num_prefix);
    pr_info("librefw: done inserting %d prefixes into bogon filter list\n", num_prefix);

    return 0;
}

