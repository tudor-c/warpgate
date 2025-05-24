#!/bin/python
import argparse
import shutil

from utils import *


def build_target(target: str, build_type: BuildType = BuildType.DEBUG):
    build_path = build_path_by_type(build_type)
    try:
        os.mkdir(build_path)
    except FileExistsError:
        pass
    command = f'cmake --build {build_path} --target {target}'
    os.system(command)

def configure_build_system(build_type: BuildType):
    build_path = build_path_by_type(build_type)
    try:
        os.makedirs(build_path)
    except FileExistsError:
        pass
    release_flag = '-DCMAKE_BUILD_TYPE=Release' if build_type is BuildType.RELEASE else ''
    os.system(f'cmake {release_flag} -S . -B {build_path}')

def clear_build_output(build_type: BuildType):
    build_path = build_path_by_type(build_type)
    try:
        shutil.rmtree(build_path)
    except FileNotFoundError:
        pass

def main():
    parser = argparse.ArgumentParser(prog='build-script')
    parser.add_argument('target', help=f'{TARGET_TRACKER} or {TARGET_CLIENT}',
                        nargs='?')
    parser.add_argument('--release', action='store_true', dest='release')
    parser.add_argument('-c', '--configure', action='store_true', dest='configure')
    parser.add_argument('--clear', action='store_true', dest='clear')
    parser.add_argument('-r', '--run', action='store_true', dest='run')

    args = parser.parse_args()
    build_type = BuildType.DEBUG if not args.release else BuildType.RELEASE

    if args.clear:
        clear_build_output(build_type)
    if args.configure:
       configure_build_system(build_type)
    if args.target is not None:
        build_target(args.target, build_type)
    if args.run:
        if args.target is None:
            print("Specify a target!")
            exit(1)
        os.system(f'./{build_path_by_type(build_type)}/{args.target}')


if __name__ == "__main__":
    main()