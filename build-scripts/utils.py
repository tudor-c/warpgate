import os
from enum import Enum

TARGET_TRACKER = 'tracker'
TARGET_CLIENT = 'client'

OUTPUT_DIR = 'output'
RESOURCES_DIR = 'resources'

DEBUG_DIR = os.path.join(OUTPUT_DIR, 'cmake-build-debug')
RELEASE_DIR = os.path.join(OUTPUT_DIR, 'cmake-build-release')


class BuildType(Enum):
    DEBUG = 0
    RELEASE = 1

def build_path_by_type(build_type: BuildType) -> str:
    return DEBUG_DIR if build_type == BuildType.DEBUG else RELEASE_DIR
