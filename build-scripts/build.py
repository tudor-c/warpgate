#!/bin/python
import argparse
import shutil
import sys

from utils import *


def build_target(target: str, build_type: BuildType = BuildType.DEBUG) -> int:
    build_path = build_path_by_type(build_type)
    try:
        os.mkdir(build_path)
    except FileExistsError:
        pass
    except FileNotFoundError:
        print("You must configure first! Run `./build --configure`")
        return 1
    command = f'cmake --build {build_path} --target {target}'
    return os.system(command)

def configure_build_system(build_type: BuildType):
    build_path = build_path_by_type(build_type)
    try:
        os.makedirs(build_path)
    except FileExistsError:
        pass
    release_flag = '-DCMAKE_BUILD_TYPE=Release' if build_type is BuildType.RELEASE else ''
    os.system(f'cmake {release_flag} -S . -B {build_path}')

def clear_build_output(build_type: BuildType):
    try:
        shutil.rmtree(OUTPUT_DIR)
    except FileNotFoundError:
        pass

def main():
    parser = argparse.ArgumentParser(prog='build-script')
    parser.add_argument('--release', action='store_true', dest='release')
    parser.add_argument('-c', '--configure', action='store_true', dest='configure')
    parser.add_argument('--clean', action='store_true', dest='clean')
    parser.add_argument('--no-build', action='store_true', dest='no_build')
    parser.add_argument('-r', '--run', action='store_true', dest='run')
    parser.add_argument('remainder', nargs='*')

    args = parser.parse_args()
    build_type = BuildType.DEBUG if not args.release else BuildType.RELEASE

    # keep in remainder only argument positioned after '--'
    if '--' in args.remainder:
        args.remainder = args.remainder[args.remainder.index('--') + 1 : ]

    if args.clean:
        clear_build_output(build_type)
    if args.configure:
       configure_build_system(build_type)
    status = 0
    if not args.no_build:
        status = build_target('warpgate', build_type)

    status_code = 0
    if args.run:
        if status != 0:
            print("Build failed, aborting run!")
            status_code = 1
        else:
            target_arguments = ' '.join(args.remainder)
            status_code = os.system(f'./{build_path_by_type(build_type)}/warpgate {target_arguments}')
    sys.exit(status_code)


if __name__ == "__main__":
    main()