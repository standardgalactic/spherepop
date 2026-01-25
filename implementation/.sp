# hard caps (blast radius)
SP_POLICY_MAX_EVENTS=200
SP_POLICY_MAX_COLLAPSE_EVENTS=0

# allowlist / denylist by relation type (LINK type=...)
SP_POLICY_DENY_LINK_TYPES="owns_root setuid chmod chown mount bind"

# deny event types entirely in bootstrap mode
SP_POLICY_DENY_EVENT_TYPES="COLLAPSE"

