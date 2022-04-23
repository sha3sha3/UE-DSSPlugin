/*
 * MIT License
 *
 * Copyright (c) 2020-2021 FrozenStorm Interactive
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "CoreMinimal.h"

class DSSLITE_API FSignalRValue
{
public:
    enum class EValueType
    {
        Number,
        Object,
        Array,
        String,
        Null,
        Boolean,
        Binary
    };

    /**
     * Create an object representing a EValueType::Null value.
     */
    FSignalRValue() :
        Type(EValueType::Null)
    {
    }

    /**
     * Create an object representing a EValueType::Null value.
     */
    FSignalRValue(std::nullptr_t) :
        Type(EValueType::Null)
    {
    }

    FSignalRValue(const int32 InValue) :
        Type(EValueType::Number)
    {
        Value.Set<NumberType>(InValue);
    }

    FSignalRValue(const uint32 InValue) :
        Type(EValueType::Number)
    {
        Value.Set<NumberType>(InValue);
    }

    FSignalRValue(const int64 InValue) :
        Type(EValueType::Number)
    {
        Value.Set<NumberType>(InValue);
    }

    FSignalRValue(const uint64 InValue) :
        Type(EValueType::Number)
    {
        Value.Set<NumberType>(InValue);
    }

    /**
     * Create an object representing a EValueType::Float with the given float value.
     */
    FSignalRValue(const float InValue) :
        Type(EValueType::Number)
    {
        Value.Set<NumberType>(InValue);
    }

    /**
     * Create an object representing a EValueType::Double with the given double value.
     */
    FSignalRValue(const double InValue) :
        Type(EValueType::Number)
    {
        Value.Set<NumberType>(InValue);
    }

    /**
     * Create an object representing a EValueType::Object with the given map of string-value's.
     */
    FSignalRValue(const TMap<FString, FSignalRValue>& InValue) :
        Type(EValueType::Object)
    {
        Value.Set<TSharedPtr<ObjectType>>(MakeShared<ObjectType>(InValue));
    }

    /**
     * Create an object representing a EValueType::Object with the given map of string-value's.
     */
    FSignalRValue(TMap<FString, FSignalRValue>&& InValue) :
        Type(EValueType::Object)
    {
        Value.Emplace<TSharedPtr<ObjectType>>(MakeShared<ObjectType>(MoveTemp(InValue)));
    }

    /**
     * Create an object representing a EValueType::Array with the given array of value's.
     */
    FSignalRValue(const TArray<FSignalRValue>& InValue) :
        Type(EValueType::Array)
    {
        Value.Set<ArrayType>(InValue);
    }

    /**
     * Create an object representing a EValueType::Array with the given array of value's.
     */
    FSignalRValue(TArray<FSignalRValue>&& InValue) :
        Type(EValueType::Array)
    {
        Value.Emplace<ArrayType>(InValue);
    }

    /**
     * Create an object representing a EValueType::String with the given string value.
     */
    FSignalRValue(const FString& InValue) :
        Type(EValueType::String)
    {
        Value.Set<StringType>(InValue);
    }

    /**
     * Create an object representing a EValueType::String with the given string value.
     */
    FSignalRValue(FString&& InValue) :
        Type(EValueType::String)
    {
        Value.Emplace<StringType>(MoveTemp(InValue));
    }

    /**
     * Create an object representing a EValueType::Boolean with the given bool value.
     */
    FSignalRValue(bool InValue) :
        Type(EValueType::Boolean)
    {
        Value.Set<BooleanType>(InValue);
    }

    /**
     * Create an object representing a value_type::binary with the given array of byte's.
     */
    FSignalRValue(const TArray<uint8>& InValue) :
        Type(EValueType::Binary)
    {
        Value.Set<BinaryType>(InValue);
    }

    /**
     * Create an object representing a value_type::binary with the given array of byte's.
     */
    FSignalRValue(TArray<uint8>&& InValue) :
        Type(EValueType::Binary)
    {
        Value.Emplace<BinaryType>(MoveTemp(InValue));
    }

    /**
     * Copies an existing value.
     */
    FSignalRValue(const FSignalRValue& OtherValue)
    {
        Type = OtherValue.Type;
        Value = OtherValue.Value;
    }

    /**
     * Moves an existing value.
     */
    FSignalRValue(FSignalRValue&& OtherValue) noexcept
    {
        Type = MoveTemp(OtherValue.Type);
        Value = MoveTemp(OtherValue.Value);
    }

    /**
     * Cleans up the resources associated with the value.
     */
    ~FSignalRValue() = default;

    /**
     * Copies an existing value.
     */
    FSignalRValue& operator=(const FSignalRValue& OtherValue)
    {
        Type = OtherValue.Type;
        Value = OtherValue.Value;
        return *this;
    }

    /**
     * Moves an existing value.
     */
    FSignalRValue& operator=(FSignalRValue&& OtherValue) noexcept
    {
        Type = MoveTemp(OtherValue.Type);
        Value = MoveTemp(OtherValue.Value);
        return *this;
    }

    /**
     * True if the object stored is a double.
     */
    FORCEINLINE bool IsDouble() const
    {
        return Type == EValueType::Number;
    }

    /**
     * True if the object stored is a Key-Value pair.
     */
    FORCEINLINE bool IsObject() const
    {
        return Type == EValueType::Object;
    }

    /**
     * True if the object stored is a string.
     */
    FORCEINLINE bool IsString() const
    {
        return Type == EValueType::String;
    }

    /**
     * True if the object stored is empty.
     */
    FORCEINLINE bool IsNull() const
    {
        return Type == EValueType::Null;
    }

    /**
     * True if the object stored is an array of value's.
     */
    FORCEINLINE bool IsArray() const
    {
        return Type == EValueType::Array;
    }

    /**
     * True if the object stored is a bool.
     */
    FORCEINLINE bool IsBoolean() const
    {
        return Type == EValueType::Boolean;
    }

    /**
     * True if the object stored is a binary blob.
     */
    FORCEINLINE bool IsBinary() const
    {
        return Type == EValueType::Binary;
    }

    FORCEINLINE EValueType GetType() const
    {
        return Type;
    }

    FORCEINLINE int64 AsInt() const
    {
        check(Type == EValueType::Number);
        return Value.Get<NumberType>();
    }

    FORCEINLINE uint64 AsUInt() const
    {
        check(Type == EValueType::Number);
        return Value.Get<NumberType>();
    }

    FORCEINLINE float AsFloat() const
    {
        check(Type == EValueType::Number);
        return Value.Get<NumberType>();
    }

    /**
     * Returns the stored object as a double. This will throw if the underlying object is not a EValueType::Double.
     */
    FORCEINLINE double AsDouble() const
    {
        check(Type == EValueType::Number);
        return Value.Get<NumberType>();
    }

    /**
     * Returns the stored object as a double. This will throw if the underlying object is not a EValueType::Double.
     */
    FORCEINLINE double AsNumber() const
    {
        check(Type == EValueType::Number);
        return Value.Get<NumberType>();
    }

    /**
     * Returns the stored object as a map of property name to value. This will throw if the underlying object is not a EValueType::Object.
     */
    FORCEINLINE const TMap<FString, FSignalRValue>& AsObject() const
    {
        check(Type == EValueType::Object);
        return *Value.Get<TSharedPtr<ObjectType>>();
    }

    /**
     * Returns the stored object as an array of value's. This will throw if the underlying object is not a EValueType::Array.
     */
    FORCEINLINE const TArray<FSignalRValue>& AsArray() const
    {
        check(Type == EValueType::Array);
        return Value.Get<ArrayType>();
    }

    /**
     * Returns the stored object as a string. This will throw if the underlying object is not a EValueType::String.
     */
    FORCEINLINE const FString& AsString() const
    {
        check(Type == EValueType::String);
        return Value.Get<StringType>();
    }

    /**
     * Returns the stored object a boolean. This will throw if the underlying object is not a EValueType::Boolean.
     */
    FORCEINLINE bool AsBool() const
    {
        check(Type == EValueType::Boolean);
        return Value.Get<BooleanType>();
    }

    /**
     * Returns the stored object as an array of bytes. This will throw if the underlying object is not EValueType::Binary.
     */
    FORCEINLINE const TArray<uint8>& AsBinary() const
    {
        check(Type == EValueType::Binary);
        return Value.Get<BinaryType>();
    }

private:
    EValueType Type;

    using NumberType = double;
    using ObjectType = TMap<FString, FSignalRValue>;
    using ArrayType = TArray<FSignalRValue>;
    using StringType = FString;
    using BooleanType = bool;
    using BinaryType = TArray<uint8>;
    using FInternalValueVariant = TVariant<NumberType, TSharedPtr<ObjectType>, ArrayType, StringType, BooleanType, BinaryType>;

    FInternalValueVariant Value;
};
