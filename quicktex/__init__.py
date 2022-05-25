try:
    from _quicktex import *
    from _quicktex import __version__
except ImportError as e:
    if 'libomp.dylib' in e.msg:
        print('\033[41m\033[01mERROR: LIBOMP NOT FOUND! PLEASE INSTALL IT WITH \033[04m`brew install libomp`\033[0m')
        print('original error message:')
    raise e
