from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.system.package_manager import Apt

require_conan_version = ">=2.0"

class AFVConan(ConanFile):
    name = "afv"
    settings = "os", "compiler", "build_type", "arch"
    version = "0.1"

    exports_sources = "cmake", "src", "CMakeLists.txt", "LICENSE"

    def system_requirements(self):
        apt = Apt(self)
        apt.install(["libncurses-dev"], check=True)

    def requirements(self):
        self.requires("fmt/10.2.1")

    def build_requirements(self):
        self.tool_requires("cmake/[^3.27]")
        self.test_requires("catch2/[^3.5.2]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = "ConanPresets.json"
        tc.generate()

        cmake = CMakeDeps(self)
        cmake.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

