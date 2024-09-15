from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class rushRecipe(ConanFile):
    name = "rush"
    version = "0.1"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Kai Patel"
    url = ""
    description = "A torrent client"
    topics = ("networking", "parsing", "torrent")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    # default_options = {
    #         "*:build_type": "Release",
    # }

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def requirements(self):
        self.requires("boost/[>=1.86 <1.87]")
        self.requires("foonathan-lexy/2022.12.1")
        self.requires("fmt/11.0.2")
        self.requires("gtest/1.15.0")
