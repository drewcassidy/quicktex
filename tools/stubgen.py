import inspect
import os.path
import tempfile
import pybind11_stubgen as sg

package = 'quicktex'
prefix = '_'

modules = set()


def find_submodules(pkg):
    modules.add(pkg.__name__.split('.')[-1])

    for element_name in dir(pkg):
        element = getattr(pkg, element_name)
        if inspect.ismodule(element):
            find_submodules(element)


if __name__ == "__main__":
    find_submodules(__import__(prefix + package))

    pkgdir = os.path.abspath(os.curdir)

    with tempfile.TemporaryDirectory() as out:

        # generate stubs using mypy Stubgen
        sg.main(['-o', out, '--root-module-suffix', "", prefix + package])

        os.curdir = pkgdir

        # walk resulting stubs and move them to their new location
        for root, dirs, files in os.walk(out):
            for stub_name in files:
                # location of the extension module's stub file
                ext_module = os.path.relpath(root, out)

                if stub_name.split('.')[-1] != 'pyi':
                    continue

                if stub_name != '__init__.pyi':
                    ext_module = os.path.join(ext_module, os.path.splitext(stub_name)[0])

                # open and read the stub file and replace all extension module names with their python counterparts
                with open(os.path.join(root, stub_name), 'r') as fp:
                    contents = fp.read()

                for mod in modules:
                    new_mod = mod.replace(prefix, '')
                    contents = contents.replace(mod, new_mod)

                # write out to the new location
                py_module = ext_module.replace(prefix, '')

                with open(os.path.join(os.curdir, *py_module.split('.'), '__init__.pyi'), 'w') as fp:
                    fp.write(contents)
                    print(ext_module + ' -> ' + fp.name)
