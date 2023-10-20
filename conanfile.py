from conan import ConanFile


class AutoDirectorRecipe(ConanFile):
    # Which settings are affecting our project?
    settings = "os", "compiler", "build_type", "arch"
    # Craft dependencies into a format that the build system understands
    generators = "PremakeDeps"

    # What do we use?
    def requirements(self):
        self.requires("tensorflow-lite/2.12.0")
        self.requires("opencv/4.5.5")
