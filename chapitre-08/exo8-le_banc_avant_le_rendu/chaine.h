#pragma once

#include <cstdint>

struct DescChaine {
    std::uint32_t imageCount = 0;
};

class ChaineEchange {
public:
    static constexpr std::uint32_t kMaxImages = 4;

    bool Create(const DescChaine &desc) noexcept {
        if (desc.imageCount == 0 || desc.imageCount > kMaxImages) {
            return false;
        }
        mDesc = desc;
        mNextImage = 0;
        mAcquired = false;
        for (std::uint32_t i = 0; i < kMaxImages; ++i) {
            mHandles[i] = 0;
        }
        mCreated = true;
        return true;
    }

    bool RegisterImage(std::uint32_t index, std::uint64_t handle) noexcept {
        if (!mCreated || index >= mDesc.imageCount) {
            return false;
        }
        mHandles[index] = handle;
        return true;
    }

    bool AcquireImage(std::uint32_t &index) noexcept {
        if (!mCreated || mAcquired) {
            return false;
        }
        mAcquired = true;
        index = mNextImage;
        return true;
    }

    bool ReleaseImage() noexcept {
        if (!mCreated || !mAcquired) {
            return false;
        }
        mAcquired = false;
        mNextImage = (mNextImage + 1u) % mDesc.imageCount;
        return true;
    }

    std::uint32_t GetLastReleasedIndex() const noexcept {
        if (!mCreated || mDesc.imageCount == 0) {
            return 0;
        }
        return (mNextImage + mDesc.imageCount - 1u) % mDesc.imageCount;
    }

    std::uint64_t GetHandle(std::uint32_t index) const noexcept {
        return index < kMaxImages ? mHandles[index] : 0;
    }

private:
    DescChaine mDesc{};
    std::uint64_t mHandles[kMaxImages] = {};
    std::uint32_t mNextImage = 0;
    bool mAcquired = false;
    bool mCreated = false;
};
