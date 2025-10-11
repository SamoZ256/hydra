#pragma once

namespace hydra::horizon {

namespace filesystem {
class Filesystem;
}

void try_install_firmware_to_filesystem(filesystem::Filesystem& fs);

} // namespace hydra::horizon
