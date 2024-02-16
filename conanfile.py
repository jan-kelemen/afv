from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeDeps, CMakeToolchain

class AFVConan(ConanFile):
    name = "afv"
    settings = "os", "compiler", "build_type", "arch"
    build_policy = "missing"

    def requirements(self):
        self.requires("fmt/10.2.1")

    def build_requirements(self):
        self.test_requires("catch2/3.5.2")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

        cmake = CMakeDeps(self)
        cmake.generate()

