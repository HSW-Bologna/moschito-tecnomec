from SCons.Script import *

"""
List directories using scons capabilities
"""


def Listdirs(path):
    result = []
    for node in Glob(f"{path}/*"):
        if isinstance(node, SCons.Node.FS.Dir):
            result.append(node)

    return result


"""
Used to support Glob() (that doesn't support recursive globbing)
to glob recursively on directories
e.g. Object([Glob(f"{dir}/*.c") for dir in Rlistdirs("lvgl/src")])
"""


def Rlistdirs(path, _result=None):
    if _result is None:
        # include also passed directory
        _result = [Dir(path)]

    for node in Listdirs(path):
        _result.append(node)
        Rlistdirs(node, _result)

    return _result
