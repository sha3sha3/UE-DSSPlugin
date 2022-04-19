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

#pragma warning (push)
#pragma warning (disable: 4582 4583)

    /**
     * Create an object representing a EValueType::Null value.
     */
	FSignalRValue() :
        Type(EValueType::Null)
	{
	    NumberValue = 0;
	}

    /**
     * Create an object representing a EValueType::Null value.
     */
    FSignalRValue(std::nullptr_t) :
        Type(EValueType::Null)
	{
	    NumberValue = 0;
	}

	FSignalRValue(const int32 Value) :
        Type(EValueType::Number)
	{
		NumberValue = Value;
	}

	FSignalRValue(const uint32 Value) :
        Type(EValueType::Number)
	{
		NumberValue = Value;
	}

	FSignalRValue(const int64 Value) :
        Type(EValueType::Number)
	{
		NumberValue = Value;
	}

	FSignalRValue(const uint64 Value) :
        Type(EValueType::Number)
	{
		NumberValue = Value;
	}

    /**
     * Create an object representing a EValueType::Float with the given float value.
     */
	FSignalRValue(const float Value) :
        Type(EValueType::Number)
	{
		NumberValue = Value;
	}

    /**
     * Create an object representing a EValueType::Double with the given double value.
     */
	FSignalRValue(const double Value) :
        Type(EValueType::Number)
	{
		NumberValue = Value;
	}

    /**
     * Create an object representing a EValueType::Object with the given map of string-value's.
     */
    FSignalRValue(const TMap<FString, FSignalRValue>& Value) :
        Type(EValueType::Object),
        ObjectValue(MakeUnique<TMap<FString, FSignalRValue>>(Value))
	{
	}

    /**
     * Create an object representing a EValueType::Object with the given map of string-value's.
     */
    FSignalRValue(TMap<FString, FSignalRValue>&& Value) :
        Type(EValueType::Object),
        ObjectValue(MakeUnique<TMap<FString, FSignalRValue>>(MoveTemp(Value)))
	{
	}

    /**
     * Create an object representing a EValueType::Array with the given array of value's.
     */
    FSignalRValue(const TArray<FSignalRValue>& Value) :
        Type(EValueType::Array),
        ArrayValue(Value)
	{
	}

    /**
     * Create an object representing a EValueType::Array with the given array of value's.
     */
    FSignalRValue(TArray<FSignalRValue>&& Value) :
        Type(EValueType::Array),
        ArrayValue(MoveTemp(Value))
	{
	}

    /**
     * Create an object representing a EValueType::String with the given string value.
     */
	FSignalRValue(const FString& Value) :
        Type(EValueType::String),
        StringValue(Value)
	{
	}

    /**
     * Create an object representing a EValueType::String with the given string value.
     */
	FSignalRValue(FString&& Value) :
        Type(EValueType::String),
        StringValue(MoveTemp(Value))
	{
	}

    /**
     * Create an object representing a EValueType::Boolean with the given bool value.
     */
    FSignalRValue(bool Value) :
        Type(EValueType::Boolean),
        BooleanValue(Value)
	{
	}

    /**
     * Create an object representing a value_type::binary with the given array of byte's.
     */
    FSignalRValue(const TArray<uint8>& Value) :
        Type(EValueType::Binary),
        BinaryValue(Value)
	{
	}

    /**
     * Create an object representing a value_type::binary with the given array of byte's.
     */
    FSignalRValue(TArray<uint8>&& Value) :
        Type(EValueType::Binary),
        BinaryValue(MoveTemp(Value))
	{
	}

    /**
     * Copies an existing value.
     */
    FSignalRValue(const FSignalRValue& OtherValue)
	{
	    Type = OtherValue.Type;
	    switch (Type)
	    {
	    case EValueType::Array:
	        new (&ArrayValue) TArray<FSignalRValue>(OtherValue.ArrayValue);
	        break;
	    case EValueType::String:
	        new (&StringValue) FString(OtherValue.StringValue);
	        break;
	    case EValueType::Number:
	        NumberValue = OtherValue.NumberValue;
	        break;
	    case EValueType::Boolean:
	        BooleanValue = OtherValue.BooleanValue;
	        break;
	    case EValueType::Object:
	        ObjectValue = MakeUnique<TMap<FString, FSignalRValue>>(*OtherValue.ObjectValue);
	        break;
	    case EValueType::Binary:
	        new (&BinaryValue) TArray<uint8>(OtherValue.BinaryValue);
	        break;
	    default:
            break;
	    }
	}

    /**
     * Moves an existing value.
     */
    FSignalRValue(FSignalRValue&& OtherValue) noexcept
	{
	    Type = MoveTemp(OtherValue.Type);
	    switch (Type)
	    {
	    case EValueType::Array:
	        new (&ArrayValue) TArray<FSignalRValue>(MoveTemp(OtherValue.ArrayValue));
	        break;
	    case EValueType::String:
	        new (&StringValue) FString(MoveTemp(OtherValue.StringValue));
	        break;
	    case EValueType::Number:
	        NumberValue = MoveTemp(OtherValue.NumberValue);
	        break;
	    case EValueType::Boolean:
	        BooleanValue = MoveTemp(OtherValue.BooleanValue);
	        break;
	    case EValueType::Object:
	        ObjectValue = MoveTemp(OtherValue.ObjectValue);
	        break;
	    case EValueType::Binary:
	        new (&BinaryValue) TArray<uint8>(MoveTemp(OtherValue.BinaryValue));
	        break;
	    default:
            break;
	    }
	}

    /**
     * Cleans up the resources associated with the value.
     */
    ~FSignalRValue()
    {
        InternalDestruct();
    }

    /**
     * Copies an existing value.
     */
    FSignalRValue& operator=(const FSignalRValue& OtherValue)
	{
	    InternalDestruct();

	    Type = OtherValue.Type;
	    switch (Type)
	    {
	    case EValueType::Array:
	        new (&ArrayValue) TArray<FSignalRValue>(OtherValue.ArrayValue);
	        break;
	    case EValueType::String:
	        new (&StringValue) FString(OtherValue.StringValue);
	        break;
	    case EValueType::Number:
	        NumberValue = OtherValue.NumberValue;
	        break;
	    case EValueType::Boolean:
	        BooleanValue = OtherValue.BooleanValue;
	        break;
	    case EValueType::Object:
	        ObjectValue = MakeUnique<TMap<FString, FSignalRValue>>(*OtherValue.ObjectValue);
	        break;
	    case EValueType::Binary:
	        new (&BinaryValue) TArray<uint8>(OtherValue.BinaryValue);
	        break;
	    default:
            break;
	    }

	    return *this;
	}

    /**
     * Moves an existing value.
     */
    FSignalRValue& operator=(FSignalRValue&& OtherValue) noexcept
	{
	    InternalDestruct();

	    Type = MoveTemp(OtherValue.Type);
	    switch (Type)
	    {
	    case EValueType::Array:
	        new (&ArrayValue) TArray<FSignalRValue>(MoveTemp(OtherValue.ArrayValue));
	        break;
	    case EValueType::String:
	        new (&StringValue) FString(MoveTemp(OtherValue.StringValue));
	        break;
	    case EValueType::Number:
	        NumberValue = MoveTemp(OtherValue.NumberValue);
	        break;
	    case EValueType::Boolean:
	        BooleanValue = MoveTemp(OtherValue.BooleanValue);
	        break;
	    case EValueType::Object:
	        ObjectValue = MoveTemp(OtherValue.ObjectValue);
	        break;
	    case EValueType::Binary:
	        new (&BinaryValue) TArray<uint8>(MoveTemp(OtherValue.BinaryValue));
	        break;
	    default:
            break;
	    }

	    return *this;
	}

#pragma warning (pop)

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
		return NumberValue;
	}

	FORCEINLINE uint64 AsUInt() const
	{
		check(Type == EValueType::Number);
		return NumberValue;
	}

	FORCEINLINE float AsFloat() const
	{
		check(Type == EValueType::Number);
		return NumberValue;
	}

    /**
     * Returns the stored object as a double. This will throw if the underlying object is not a EValueType::Double.
     */
	FORCEINLINE double AsDouble() const
	{
		check(Type == EValueType::Number);
		return NumberValue;
	}

    /**
     * Returns the stored object as a double. This will throw if the underlying object is not a EValueType::Double.
     */
    FORCEINLINE double AsNumber() const
	{
	    check(Type == EValueType::Number);
	    return NumberValue;
	}

    /**
     * Returns the stored object as a map of property name to value. This will throw if the underlying object is not a EValueType::Object.
     */
    FORCEINLINE const TMap<FString, FSignalRValue>& AsObject() const
	{
	    check(Type == EValueType::Object);
	    return *ObjectValue;
	}

    /**
     * Returns the stored object as an array of value's. This will throw if the underlying object is not a EValueType::Array.
     */
    FORCEINLINE const TArray<FSignalRValue>& AsArray() const
	{
	    check(Type == EValueType::Array);
	    return ArrayValue;
	}

    /**
     * Returns the stored object as a string. This will throw if the underlying object is not a EValueType::String.
     */
    FORCEINLINE const FString& AsString() const
	{
	    check(Type == EValueType::String);
	    return StringValue;
	}

    /**
     * Returns the stored object a boolean. This will throw if the underlying object is not a EValueType::Boolean.
     */
    FORCEINLINE bool AsBool() const
	{
	    check(Type == EValueType::Boolean);
	    return BooleanValue;
	}

    /**
     * Returns the stored object as an array of bytes. This will throw if the underlying object is not EValueType::Binary.
     */
    FORCEINLINE const TArray<uint8>& AsBinary() const
	{
	    check(Type == EValueType::Binary);
	    return BinaryValue;
	}

private:
    FORCEINLINE void InternalDestruct()
    {
        switch (Type)
        {
        case EValueType::Array:
            ArrayValue.~TArray();
            break;
        case EValueType::String:
            StringValue.~FString();
            break;
        case EValueType::Object:
            ObjectValue.Reset();
            break;
        case EValueType::Binary:
            BinaryValue.~TArray();
            break;
        default:
            break;
        }
    }

	EValueType Type;
	union
	{
		double NumberValue;
	    TUniquePtr<TMap<FString, FSignalRValue>> ObjectValue;
	    TArray<FSignalRValue> ArrayValue;
	    FString StringValue;
	    bool BooleanValue;
	    TArray<uint8> BinaryValue;
	};
};
