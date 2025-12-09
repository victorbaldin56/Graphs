import argparse
import os
import subprocess
import glob

CURRENT_PATH = os.path.abspath(os.path.dirname(__file__))
PATH_TO_INPUT = os.path.join(CURRENT_PATH, "in")


def get_dot_file_path(input_path):
  return os.path.join(
    (os.path.dirname(input_path)), os.path.pardir) + "/out/" + (
      os.path.basename(input_path).replace('.in', '.dot'))


def get_reference_dot(ans_file):
  with open(ans_file, 'r') as af:
    return af.read()


def test(executable):
  fail = False

  for input_path in glob.glob(os.path.join(PATH_TO_INPUT, '*')):
    ans_path = get_dot_file_path(input_path)
    with open(input_path, 'r') as input_file:
      process = subprocess.run([executable],
                               stdin=input_file,
                               text=True,
                               capture_output=True)
      if process.returncode != 0:
        raise RuntimeError(
          f'Driver failed on test {input_path}: {process.stderr}')
      output = process.stdout
      ref_output = get_reference_dot(ans_path)
      if (output != ref_output):
        print(f"Test {input_path} failed\n"
              f"Expected: {ref_output}\n"
              f"Actual: {output}\n")
        fail = True
      else:
        print(f"Test {ans_path} passed")

  if fail:
    raise RuntimeError("End-to-end test failed\n")


if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument('--executable', help='Executable to test')
  args = parser.parse_args()
  test(args.executable)
