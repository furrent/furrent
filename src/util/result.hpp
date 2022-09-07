/**
 * @file result.hpp
 * @author Filippo Ziche
 * @version 0.1
 * @date 2022-09-02
 */

#pragma once

#include <variant>

namespace fur::util {


/// Possible errors that can occur during
enum class Error {
  // --- Peer
  PeerAnnounceTracker,

  // --- Strategy
  /// Occurs when there was no element to extract in a strategy
  StrategyEmpty,

  // --- Channel
  /// Occurs when worker was waiting for an item but the Channel
  /// stopped serving
  ChannelStoppedServing,
  /// Occurs when the strategy used returned nothing
  ChannelStrategyFailed,
  /// Occurs when we try to extract an item but the Channel was empty
  ChannelEmpty,

  // --- Bencode
  /// The bencoded string is not valid
  DecodeInvalidString,
  /// A integer was not in the form ['i', 'number', 'e']
  DecodeIntFormat,
  /// The integer was not a valid integer
  DecodeIntValue,
  /// A string was not in the form ['length', ':', 'string']
  DecodeStringFormat,
  /// A string has a length that is not a valid integer
  DecodeStringLength,
  /// A list was not in the form ['l', 'bencode_value', ... , 'e']
  DecodeListFormat,
  /// A dictionary was not in the form ['d', 'bencode_value', ... , 'e']
  DecodeDictFormat,
  /// A dictionary key was not a string
  DecodeDictKey,
  /// The keys of the dictionary were not in lexicographical order
  DecodeDictKeyOrder,

  // --- Policy
  /// There are no more elements
  PolicyEmpty,
  /// Policy returned no element
  PolicyFailure
};

/// @brief Used to handle errors without using exceptions
/// @tparam R type of the result
template<typename R>
class Result {

    std::variant<R, Error> _inner;

    explicit Result(R&& result);
    explicit Result(E&& error);

public:

    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;
    
    Result(Result&&) noexcept;
    Result& operator=(Result&&) noexcept;

    /// @return True if there is no error. 
    [[nodiscard]] bool valid() const;
    /// @return Result if it is present, undefined behaviour otherwise
    R& operator *();

    /// @return Result if it is present, undefined behaviour otherwise
    R* operator ->();
    /// @return error that occurred 
    [[nodiscard]] const Error& error() const;

public:
    /// Creates an ok result
    static Result OK(R&& result);
    /// Creates an error result 
    static Result ERROR(Error&& error);
};

} // namespace fur::util

#include <util/result.inl>