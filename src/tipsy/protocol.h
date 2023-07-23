#pragma once
#ifndef TIPSY_ENCODER_PROTOCOL_H
#define TIPSY_ENCODER_PROTOCOL_H
/*
 * Given an encoder for raw binary, we need a communications protocol. This sort of
 * lays one out with a stateful encoder and decoder object.
 *
 * We have to take great care to have these be audio thread safe, so heap allocation
 * and copies are things we try hard not to do. Please carefully read the comments on
 * functions to make sure you get the ownership correct.
 */

#include <cstdint>
#include <cstring>
#include "binary-to-float.h"
#include "version.h"

#if __cplusplus >= 201703L
#define TIPSY_NODISCARD [[nodiscard]]
#else
#define TIPSY_NODISCARD
#endif

namespace tipsy
{

// We know that our 3-byte-to-binary encoder encodes numbers strictly in the range
// -1,1 so any valid float outside that range can be used as a sentinel. We use floats
// in the '3' range here.
static constexpr float kMessageBeginSentinel{3.1f};
static constexpr float kVersionSentinel{3.2f};
static constexpr float kSizeSentinel{3.3f};
static constexpr float kMimeTypeSentinel{3.4f};
static constexpr float kBodySentinel{3.5f};
static constexpr float kEndMessageSentinel{3.6f};

static constexpr uint16_t kVersion{0x01};

// limits
static constexpr size_t kMaxMimeTypeSize{256};
static constexpr size_t kMaxMessageLength{1 << 23};

inline bool isValidSentinel(float f) noexcept
{
    return (f == kMessageBeginSentinel) || (f == kVersionSentinel) || (f == kSizeSentinel) ||
           (f == kMimeTypeSentinel) || (f == kBodySentinel) || (f == kEndMessageSentinel);
}
inline bool isValidProtocolEncoding(float f) noexcept
{
    return isValidDataEncoding(f) || isValidSentinel(f);
}
inline std::string sentinelDisplayName(float f) noexcept
{
    if (!isValidSentinel(f))
        return "NOT_A_SENTINEL";
#define CK(s)                                                                                      \
    if (f == s)                                                                                    \
        return #s;
    CK(kMessageBeginSentinel);
    CK(kVersionSentinel);
    CK(kSizeSentinel);
    CK(kMimeTypeSentinel);
    CK(kBodySentinel);
    CK(kEndMessageSentinel);
#undef CK

    return "ERROR";
}

struct ProtocolEncoder
{
    enum class EncoderResult : uint16_t
    {
        DORMANT = 1,
        ENCODING_MESSAGE,
        MESSAGE_COMPLETE,
        MESSAGE_TERMINATED,
        MESSAGE_INITIATED,

        ERROR_UNKNOWN = 0x100,
        ERROR_NO_MESSAGE_ACTIVE,
        ERROR_MESSAGE_TOO_LARGE,
        ERROR_MIME_TYPE_TOO_LARGE,
        ERROR_MESSAGE_ALREADY_ACTIVE,
        ERROR_MISSING_MIME_TYPE,
        ERROR_MISSING_DATA,
    };

    bool isError(EncoderResult r) const { return r >= EncoderResult::ERROR_UNKNOWN; }

    TIPSY_NODISCARD
    EncoderResult initiateMessage(const char *inMimeType, uint32_t inDataBytes,
                                  const unsigned char *const inData)
    {
        assert(kMessageBeginSentinel > tipsy::maximumEncodedFloat());

        if (inDataBytes > kMaxMessageLength)
        {
            return EncoderResult::ERROR_MESSAGE_TOO_LARGE;
        }
        if ((inDataBytes > 0) && (nullptr == inData))
        {
            return EncoderResult::ERROR_MISSING_DATA;
        }

        if (nullptr == inMimeType)
        {
            return EncoderResult::ERROR_MISSING_MIME_TYPE;
        }
        auto ms = strlen(inMimeType) + 1;
        if (ms > kMaxMimeTypeSize)
        {
            return EncoderResult::ERROR_MIME_TYPE_TOO_LARGE;
        }
        if (!isDormant())
        {
            return EncoderResult::ERROR_MESSAGE_ALREADY_ACTIVE;
        }

        mimeType = inMimeType;
        mimeTypeSize = ms;
        data = inData;
        dataBytes = inDataBytes;

        setState(EncoderState::START_MESSAGE);

        return EncoderResult::MESSAGE_INITIATED;
    }

    TIPSY_NODISCARD
    EncoderResult getNextMessageFloat(float &f)
    {
        switch (encoderState)
        {
        case EncoderState::NO_MESSAGE:
            f = 0;
            return EncoderResult::DORMANT;
            break;
        case EncoderState::START_MESSAGE:
        {
            f = kMessageBeginSentinel;
            pos++;
            if (pos == 3)
            {
                setState(EncoderState::HEADER_VERSION);
            }
            return EncoderResult::ENCODING_MESSAGE;
            break;
        }
        case EncoderState::HEADER_VERSION:
        {
            if (pos == 0)
            {
                f = kVersionSentinel;
                pos++;
            }
            else
            {
                f = FloatBytes(kVersion);
                setState(EncoderState::HEADER_SIZE);
            }
            return EncoderResult::ENCODING_MESSAGE;
        }
        break;
        case EncoderState::HEADER_SIZE:
        {
            if (pos == 0)
            {
                f = kSizeSentinel;
                pos++;
            }
            else
            {
                f = FloatBytes(dataBytes);
                setState(EncoderState::HEADER_MIMETYPE);
            }
            return EncoderResult::ENCODING_MESSAGE;
        }
        break;
        case EncoderState::HEADER_MIMETYPE:
        {
            if (pos == 0)
            {
                f = kMimeTypeSentinel;
                pos++;
            }
            else if (pos == 1)
            {
                f = FloatBytes(mimeTypeSize);
                pos++;
            }
            else
            {
                auto dp = (uint32_t)(pos - 2);
                if (dp + 3 < mimeTypeSize)
                {
                    auto mt = (unsigned char *)(mimeType + dp);
                    f = FloatBytes(mt[0], mt[1], mt[2]);

                    pos += 3;
                    if (pos - 1 == mimeTypeSize)
                    {
                        setState(EncoderState::BODY);
                    }
                }
                else
                {
                    char d[3]{0, 0, 0};
                    int i{0};
                    for (; dp < mimeTypeSize; ++dp, ++i)
                    {
                        d[i] = mimeType[dp];
                    }

                    f = FloatBytes(d[0], d[1], d[2]);
                    setState(EncoderState::BODY);
                }
            }
            return EncoderResult::ENCODING_MESSAGE;
        }
        break;
        case EncoderState::BODY:
        {
            if (pos == 0)
            {
                f = kBodySentinel;
                pos++;
            }
            else if (pos == dataBytes + 1)
            {
                setState(EncoderState::END_MESSAGE);
            }
            else if (pos + 2 < dataBytes)
            {
                auto d = data + pos - 1;
                f = FloatBytes(d[0], d[1], d[2]);
                pos += 3;
                if (pos - 1 == dataBytes)
                {
                    setState(EncoderState::END_MESSAGE);
                }
            }
            else
            {
                unsigned char d[3]{0, 0, 0};
                int i{0};
                for (unsigned int dpos = pos - 1; dpos < dataBytes; ++dpos, ++i)
                {
                    d[i] = data[dpos];
                }
                f = FloatBytes(d[0], d[1], d[2]);
                setState(EncoderState::END_MESSAGE);
            }
            return EncoderResult::ENCODING_MESSAGE;
        }
        break;
        case EncoderState::END_MESSAGE:
        {
            f = kEndMessageSentinel;
            setState(EncoderState::NO_MESSAGE);
            return EncoderResult::MESSAGE_COMPLETE;
        }
        break;
        }

        return EncoderResult::ERROR_UNKNOWN;
    }

    TIPSY_NODISCARD
    EncoderResult terminateCurrentMessage()
    {
        if (encoderState == EncoderState::NO_MESSAGE)
        {
            return EncoderResult::ERROR_NO_MESSAGE_ACTIVE;
        }
        setState(EncoderState::NO_MESSAGE);
        return EncoderResult::MESSAGE_TERMINATED;
    }

    bool isDormant() { return encoderState == EncoderState::NO_MESSAGE; }

  private:
    const char *mimeType{nullptr};
    uint32_t dataBytes{0};
    uint16_t mimeTypeSize{0};
    const unsigned char *data{nullptr};

    enum class EncoderState : uint16_t
    {
        NO_MESSAGE,
        START_MESSAGE,
        HEADER_VERSION,
        HEADER_SIZE,
        HEADER_MIMETYPE,
        BODY,
        END_MESSAGE
    } encoderState{EncoderState::NO_MESSAGE};

    unsigned int pos{0};

    void setState(EncoderState s)
    {
        encoderState = s;
        pos = 0;
    }
};

struct ProtocolDecoder
{
    enum class DecoderResult : uint16_t
    {
        DORMANT = 1,
        PARSING_HEADER,
        HEADER_READY,
        PARSING_BODY,
        BODY_READY,

        ERROR_UNKNOWN = 0x100,
        ERROR_INCOMPATIBLE_VERSION,
        ERROR_MALFORMED_HEADER,
        ERROR_DATA_TOO_LARGE
    };

    static bool isError(DecoderResult r) { return r >= DecoderResult::ERROR_UNKNOWN; }

    bool provideDataBuffer(unsigned char *data, uint32_t size)
    {
        if (decoderState == DecoderState::START_BODY)
            return false;

        dataStore = data;
        dataStoreSize = size;
        return true;
    }

    const char *getMimeType() const { return mimetype; }
    uint32_t getDataSize() const { return dataSize; }

    TIPSY_NODISCARD
    DecoderResult readFloat(float f)
    {
        assert(kMessageBeginSentinel > tipsy::maximumEncodedFloat());

        if (f == kMessageBeginSentinel)
        {
            setState(DecoderState::START_HEADER);
            dataSize = 0;
            memset(mimetype, 0, sizeof(mimetype));
            version = -1;
            return DecoderResult::PARSING_HEADER;
        }

        // Use sentinels to force state for next read
        if (f == kVersionSentinel)
        {
            setState(DecoderState::START_VERSION);
            return DecoderResult::PARSING_HEADER;
        }
        if (f == kSizeSentinel)
        {
            setState(DecoderState::START_SIZE);
            return DecoderResult::PARSING_HEADER;
        }
        if (f == kMimeTypeSentinel)
        {
            setState(DecoderState::START_MIMETYPE);
            return DecoderResult::PARSING_HEADER;
        }
        if (f == kBodySentinel)
        {
            setState(DecoderState::START_BODY);
            return DecoderResult::HEADER_READY;
        }
        if (f == kEndMessageSentinel)
        {
            setState(DecoderState::DOING_NOTHING);
            return DecoderResult::BODY_READY;
        }

        switch (decoderState)
        {
        case DecoderState::DOING_NOTHING:
            return DecoderResult::DORMANT;
        case DecoderState::START_HEADER:
            return DecoderResult::PARSING_HEADER;
        case DecoderState::START_VERSION:
            if (pos == 0)
            {
                version = uint16_FromFloat(f);
                pos++;
                if (version > 0 && version <= kVersion)
                {
                    return DecoderResult::PARSING_HEADER;
                }
                else
                {
                    return DecoderResult::ERROR_INCOMPATIBLE_VERSION;
                }
            }
            else
            {
                return DecoderResult::ERROR_MALFORMED_HEADER;
            }
            break;

        case DecoderState::START_SIZE:
            if (pos == 0)
            {
                dataSize = uint32_FromFloat(f);
                if (dataSize >= dataStoreSize)
                    return DecoderResult::ERROR_DATA_TOO_LARGE;
                pos++;
                return DecoderResult::PARSING_HEADER;
            }
            else
            {
                return DecoderResult::ERROR_MALFORMED_HEADER;
            }
            break;

        case DecoderState::START_MIMETYPE:
        {
            if (pos == 0)
            {
                mimetypeSize = uint16_FromFloat(f);
                pos++;
                return DecoderResult::PARSING_HEADER;
            }
            else
            {
                if (pos > mimetypeSize)
                {
                    return DecoderResult::ERROR_MALFORMED_HEADER;
                }
                if (pos >= kMaxMimeTypeSize - 4)
                {
                    return DecoderResult::ERROR_DATA_TOO_LARGE;
                }

                auto wp = pos - 1;
                auto float_bytes = FloatBytes(f);
                mimetype[wp] = float_bytes.first();
                mimetype[wp + 1] = float_bytes.second();
                mimetype[wp + 2] = float_bytes.third();

                pos += 3;
                return DecoderResult::PARSING_HEADER;
            }
            break;
        }
        case DecoderState::START_BODY:
            if (pos < dataSize - 3)
            {
                auto float_bytes = FloatBytes(f);
                dataStore[pos++] = float_bytes.first();
                dataStore[pos++] = float_bytes.second();
                dataStore[pos++] = float_bytes.third();
                return DecoderResult::PARSING_BODY;
            }
            else if (pos < dataSize && pos < dataStoreSize)
            {
                auto float_bytes = FloatBytes(f);
                int i = 0;
                while (pos < dataSize && pos < dataStoreSize)
                {
                    if (i == 0)
                        dataStore[pos++] = float_bytes.first();
                    else if (i == 1)
                        dataStore[pos++] = float_bytes.second();
                    else
                        dataStore[pos++] = float_bytes.third();
                    i++;
                }
                if (pos == dataSize)
                    return DecoderResult::PARSING_BODY;
                return DecoderResult::ERROR_DATA_TOO_LARGE;
            }
            else
            {
                return DecoderResult::ERROR_DATA_TOO_LARGE;
            }
            break;
        }

        return DecoderResult::ERROR_UNKNOWN;
    }

  private:
    enum class DecoderState : uint8_t
    {
        DOING_NOTHING,
        START_VERSION,
        START_HEADER,
        START_SIZE,
        START_MIMETYPE,
        START_BODY
    } decoderState{DecoderState::DOING_NOTHING};

    uint32_t pos{0};
    uint16_t version;
    uint32_t dataSize;
    char mimetype[kMaxMimeTypeSize];
    uint16_t mimetypeSize;

    unsigned char *dataStore{nullptr};
    uint32_t dataStoreSize{0};

    void setState(DecoderState s)
    {
        decoderState = s;
        pos = 0;
    }
};

// convenience shorthands for client code
using EncoderResult = ProtocolEncoder::EncoderResult;
using DecoderResult = ProtocolDecoder::DecoderResult;

} // namespace tipsy
#endif // TIPSY_ENCODER_PROTOCOL_H
