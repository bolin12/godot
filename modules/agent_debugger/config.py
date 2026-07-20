def can_build(env, platform):
    return env.debug_features


def configure(env):
    pass


def is_enabled():
    # Experimental research module. Enable explicitly when an Agent debugging build is needed.
    return False
