#pragma once

#include "common/io/stream.hpp"

namespace hydra::io {

class StreamView : public IStream {
  public:
    StreamView(IStream* base_, u64 offset_, u64 size_)
        : base{base_}, offset{offset_}, size{size_} {
        base->SeekTo(offset);
    }
    StreamView(IStream* base_, u64 offset_)
        : StreamView(base_, offset_, base_->GetSize() - offset_) {}

    u64 GetSeek() const override { return base->GetSeek() - offset; }
    void SeekTo(u64 seek) override { base->SeekTo(offset + seek); }
    void SeekBy(u64 offset_) override { base->SeekBy(offset_); }

    u64 GetSize() const override { return size; }

    void Flush() override { base->Flush(); }

    void ReadRaw(std::span<u8> buffer) override { base->ReadRaw(buffer); }
    void WriteRaw(std::span<const u8> buffer) override {
        base->WriteRaw(buffer);
    }
    u8* ConsumePtrRaw(usize size_) override {
        return base->ConsumePtrRaw(size_);
    }

  protected:
    IStream* base;

  private:
    u64 offset;
    u64 size;
};

// TODO: remove
class OwnedStreamView : public StreamView {
  public:
    using StreamView::StreamView;

    ~OwnedStreamView() override { delete base; }

    ZTD_MAKE_NON_COPYABLE(OwnedStreamView);
};

} // namespace hydra::io
