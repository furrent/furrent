#include <strategy/local.hpp>

#include <torrent_manager.hpp>

namespace fur::strategy {

void LocalStrategy::insert(PieceDescriptor descriptor, std::list<PieceDescriptor>& descriptors) {
    descriptors.push_back(descriptor);
}

} // namespace fur::strategy