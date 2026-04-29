create table hcf_ipv4_history(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ip_addr BLOB NOT NULL CHECK (length(ip_addr) = 4),
    ip_addr_str TEXT NOT NULL,
    ttl INTEGER NOT NULL,
    hc INTEGER NOT NULL,
    UNIQUE(ip_addr)
);

create table hcf_ipv6_history(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ip_addr BLOB NOT NULL CHECK (length(ip_addr) = 16),
    ip_addr_str TEXT,
    ttl INTEGER,
    hc INTEGER,
    UNIQUE(ip_addr)
);
