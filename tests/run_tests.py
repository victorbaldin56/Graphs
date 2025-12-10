import argparse
import os
import subprocess
import tempfile
import glob

CURRENT_PATH = os.path.abspath(os.path.dirname(__file__))
PATH_TO_INPUT = os.path.join(CURRENT_PATH, "in")


def get_dot_file_path(input_path, output_path):
  return os.path.join(os.path.dirname(input_path), os.path.pardir, output_path,
                      os.path.basename(input_path).replace('.in', '.dot'))


def read_file(ans_file):
  with open(ans_file, 'r') as af:
    return af.read()


def test(executable):
  fail = False

  for input_path in glob.glob(os.path.join(PATH_TO_INPUT, '*')):
    ans_path = get_dot_file_path(input_path, "out")
    dom_path = get_dot_file_path(input_path, "dom")
    pdom_path = get_dot_file_path(input_path, "pdom")

    with open(input_path, 'r') as input_file:
      with tempfile.NamedTemporaryFile() as dom, tempfile.NamedTemporaryFile(
      ) as pdom:
        process = subprocess.run(
          [executable, f"--domtree={dom.name}", f"--pdomtree={pdom.name}"],
          stdin=input_file,
          text=True,
          capture_output=True)
        if process.returncode != 0:
          raise RuntimeError(
            f'Driver failed on test {input_path}: {process.stderr}')

        output = process.stdout
        odom = read_file(dom.name)
        opdom = read_file(pdom.name)
        ref_output = read_file(ans_path)
        ref_dom = read_file(dom_path)
        ref_pdom = read_file(pdom_path)

        if (output != ref_output):
          print(f"Test {input_path} failed: graph")
          fail = True
        if (odom != ref_dom):
          print(f"Test {input_path} failed: dominators")
          fail = True
        if (opdom != ref_pdom):
          print(f"Test {input_path} failed: postdominators")
          fail = True
        else:
          print(f"Test {ans_path} passed")

  if fail:
    raise RuntimeError("End-to-end test failed")


if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument('--executable', help='Executable to test')
  args = parser.parse_args()
  test(args.executable)
