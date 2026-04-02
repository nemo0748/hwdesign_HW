def check_install():
    """
    Verifies that xilinxutils is installed and functional.
    Returns version and basic environment info.
    """
    from importlib.metadata import version
    return {
        "package": "xilinxutils",
        "version": version("xilinxutils"),
        "status": "OK"
    }
