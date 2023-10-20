import subprocess


def run_conan(build_type):
    subprocess.run(
        (
            "conan",
            "install",
            ".",
            "--build",
            "missing",
            "--output-folder=./dependencies",
            f"--settings=build_type={build_type}",
        )
    )


def run_premake(action):
    subprocess.run(
        (
            "premake5",
            action,
        )
    )


if __name__ == "__main__":
    run_conan("Debug")
    run_conan("Release")
    run_premake("vs2022")
