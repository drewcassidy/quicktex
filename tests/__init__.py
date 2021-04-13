import os.path

# Some checks to run before tests can begin
images_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'images')

assert os.path.isdir(images_path), 'test images repo not present'
assert os.path.isfile(os.path.join(images_path, '__init__.py')), 'images __init__.py not present, is the test image repo present?'
bp_size = os.path.getsize(os.path.join(images_path, 'Boilerplate.png'))
assert bp_size == 955989, 'Boilerplate.png is the wrong size, is the test image repo checked out with LFS enabled?'
