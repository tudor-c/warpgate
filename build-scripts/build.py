#!/bin/python
import argparse
import shutil
import subprocess
import sys

from utils import *


def run_command(command: str, verbose: bool = False) -> int:
    if verbose:
        print(f"Executing: {command}")

    try:
        result = subprocess.run(command, shell=True, check=False,
            stdout=subprocess.PIPE if not verbose else None,
            stderr=subprocess.PIPE if not verbose else None)
        if result.returncode != 0 and not verbose:
            print(f"Command failed with exit code {result.returncode}: {command}")
            print("STDOUT:", result.stdout.decode() if result.stdout else "")
            print("STDERR:", result.stderr.decode() if result.stderr else "")
        return result.returncode
    except Exception as e:
        print(f"Failed to execute command: {command}")
        print(f"Error: {str(e)}")
        return 1


def build_target(target: str, build_type: BuildType = BuildType.DEBUG, verbose: bool = False) -> int:
    build_path = build_path_by_type(build_type)
    try:
        os.mkdir(build_path)
    except FileExistsError:
        pass
    except FileNotFoundError:
        print("You must configure first! Run `./build --configure`")
        return 1

    command = f'cmake --build {build_path} --target {target} -- -j$(nproc)'
    return run_command(command, verbose)


def configure_build_system(build_type: BuildType, verbose: bool = False):
    build_path = build_path_by_type(build_type)
    try:
        os.makedirs(build_path)
    except FileExistsError:
        pass

    release_flag = '-DCMAKE_BUILD_TYPE=Release' if build_type is BuildType.RELEASE else ''
    return run_command(f'cmake {release_flag} -S . -B {build_path}', verbose)


def clear_build_output(build_type: BuildType):
    build_path = build_path_by_type(build_type)
    try:
        if os.path.exists(build_path):
            print(f"Removing build directory: {build_path}")
            shutil.rmtree(build_path)
        if os.path.exists(OUTPUT_DIR) and OUTPUT_DIR != build_path:
            print(f"Removing output directory: {OUTPUT_DIR}")
            shutil.rmtree(OUTPUT_DIR)
    except Exception as e:
        print(f"Error during cleanup: {str(e)}")

def main():
    parser = argparse.ArgumentParser(prog='build-script')
    parser.add_argument('--release', action='store_true', dest='release')
    parser.add_argument('-c', '--configure', action='store_true', dest='configure')
    parser.add_argument('--clean', action='store_true', dest='clean')
    parser.add_argument('--no-build', action='store_true', dest='no_build')
    parser.add_argument('-r', '--run', action='store_true', dest='run')
    parser.add_argument('-v', '--verbose', action='store_true', dest='verbose')
    parser.add_argument('remainder', nargs='*')

    args = parser.parse_args()
    build_type = BuildType.DEBUG if not args.release else BuildType.RELEASE

    # keep in remainder only argument positioned after '--'
    if '--' in args.remainder:
        args.remainder = args.remainder[args.remainder.index('--') + 1 : ]

    if args.clean:
        clear_build_output(build_type)

    if args.configure:
        status = configure_build_system(build_type, args.verbose)
        if status != 0:
            print("Configuration failed!")
            sys.exit(status)

    status = 0
    if not args.no_build:
        status = build_target('warpgate', build_type, args.verbose)

    status_code = 0
    if args.run:
        if status != 0:
            print("Build failed, aborting run!")
            status_code = 1
        else:
            target_arguments = ' '.join(args.remainder)
            status_code = run_command(f'./{build_path_by_type(build_type)}/warpgate {target_arguments}', args.verbose)
    sys.exit(status_code)


if __name__ == "__main__":
    main()