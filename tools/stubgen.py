import inspect
import os.path
import tempfile
import mypy.stubgen as sg

package = 'quicktex'
prefix = '_'

modules = set()


def find_submodules(mod):
    print(mod.__name__)
    modules.add(mod.__name__.split('.')[-1])

    for element_name in dir(mod):
        element = getattr(mod, element_name)
        if inspect.ismodule(element):
            find_submodules(element)


if __name__ == "__main__":
    find_submodules(__import__(prefix + package))
    with tempfile.TemporaryDirectory() as out:
        sg.generate_stubs(sg.parse_options(['-o', out, '-p', prefix + package]))
        print(os.path.abspath(os.curdir))
        for root, dirs, files in os.walk(out):
            for stub_name in files:
                module_path = os.path.relpath(root, out)

                if stub_name != '__init__.pyi':
                    module_path = os.path.join(module_path, os.path.splitext(stub_name)[0])

                module_name = module_path.replace(prefix, '')

                with open(os.path.join(root, stub_name), 'r') as fp:
                    contents = fp.read()

                for mod in modules:
                    new_mod = mod.replace(prefix, '')
                    contents.replace(mod, new_mod)

                with open(os.path.join(os.curdir, *module_name.split('.'), '__init__.pyi'), 'w') as fp:
                    fp.write(contents)
