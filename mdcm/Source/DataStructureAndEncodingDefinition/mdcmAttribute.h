/*********************************************************
 *
 * MDCM
 *
 * Modifications github.com/issakomi
 *
 *********************************************************/

/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2011 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef MDCMATTRIBUTE_H
#define MDCMATTRIBUTE_H

#include "mdcmTypes.h"
#include "mdcmVR.h"
#include "mdcmTagToType.h"
#include "mdcmVM.h"
#include "mdcmElement.h"
#include "mdcmDataElement.h"
#include "mdcmDataSet.h"
#include <string>
#include <vector>
#include <sstream>
#include <utility>

namespace mdcm
{

#if 0
template <int T>
class VRVLSize;

// VL is coded on 16 bits
template <>
class VRVLSize<0>
{
public:
  static inline uint16_t
  Read(std::istream & _is)
  {
    uint16_t l;
    _is.read((char *)&l, 2);
    return l;
  }
  static inline void
  Write(std::ostream & os)
  {
    (void)os;
  }
};

// VL is coded on 32 bits
template <>
class VRVLSize<1>
{
public:
  static inline uint32_t
  Read(std::istream & _is)
  {
    char dummy[2];
    _is.read(dummy, 2);
    uint32_t l;
    _is.read((char *)&l, 4);
    return l;
  }
  static inline void
  Write(std::ostream & os)
  {
    (void)os;
  }
};
#endif

/*
 * Attribute class
 * This class use template metaprograming tricks to let the user know when the template
 * instanciation does not match the public dictionary.
 *
 * Attribute<0x0018, 0x1182, VR::IS, VM::VM1> fd1 = {}; // not enough parameters
 * Attribute<0x0018, 0x1182, VR::IS, VM::VM2> fd2 = {0, 1, 2}; // too many initializers
 *
 * VR must be compatible with the public dictionary
 *
 * VM validation is limited to attributes with fixed VM, TODO validate all VMs
 *
 */

template <uint16_t  Group,
          uint16_t  Element,
          long long TVR = TagToType<Group, Element>::VRType,
          int       TVM = TagToType<Group, Element>::VMType>
class Attribute
{
public:
  typedef typename VRToType<TVR>::Type ArrayType;

  enum
  {
    VMType = VMToLength<TVM>::Length
  };

  ArrayType Internal[VMToLength<TVM>::Length];

  static_assert(static_cast<bool>(static_cast<VR::VRType>(TVR) & static_cast<VR::VRType>(TagToType<Group, Element>::VRType)), "");
  static_assert(
    (static_cast<unsigned int>(TagToType<Group, Element>::VMType) > static_cast<unsigned int>(VM::VM_FIXED_LENGTH)) ||
    ((static_cast<unsigned int>(TagToType<Group, Element>::VMType) < static_cast<unsigned int>(VM::VM_FIXED_LENGTH)) &&
    (TVM == static_cast<unsigned int>(TagToType<Group, Element>::VMType))), "");

  static Tag
  GetTag()
  {
    return Tag(Group, Element);
  }

  static VR
  GetVR()
  {
    return static_cast<VR::VRType>(TVR);
  }

  static VM
  GetVM()
  {
    return static_cast<VM::VMType>(TVM);
  }

  // The following two methods do make sense only in case of public element,
  // when the template is intanciated with private element the VR/VM are simply
  // defaulted to allow everything (see mdcmTagToType.h default template for TagToType)
  static VR
  GetDictVR()
  {
    return static_cast<VR::VRType>(TagToType<Group, Element>::VRType);
  }

  static VM
  GetDictVM()
  {
    return static_cast<VM::VMType>(TagToType<Group, Element>::VMType);
  }

  unsigned int
  GetNumberOfValues() const
  {
    return VMToLength<TVM>::Length;
  }

  void
  Print(std::ostream & os) const
  {
    os << GetTag() << ' '
       << TagToType<Group, Element>::GetVRString() << ' '
       << TagToType<Group, Element>::GetVMString() << ' '
       << Internal[0];
    for (unsigned int i = 1; i < GetNumberOfValues(); ++i)
      os << ',' << Internal[i];
  }

  bool
  operator==(const Attribute & att) const
  {
    return std::equal(Internal, Internal + GetNumberOfValues(), att.GetValues());
  }

  bool
  operator!=(const Attribute & att) const
  {
    return !std::equal(Internal, Internal + GetNumberOfValues(), att.GetValues());
  }

  bool
  operator<(const Attribute & att) const
  {
    return std::lexicographical_compare(
      Internal, Internal + GetNumberOfValues(), att.GetValues(), att.GetValues() + att.GetNumberOfValues());
  }

  ArrayType &
  GetValue(unsigned int idx = 0)
  {
    assert(idx < GetNumberOfValues());
    return Internal[idx];
  }

  ArrayType & operator[](unsigned int idx) { return GetValue(idx); }

  const ArrayType &
  GetValue(unsigned int idx = 0) const
  {
    assert(idx < GetNumberOfValues());
    return Internal[idx];
  }

  const ArrayType & operator[](unsigned int idx) const { return GetValue(idx); }

  void
  SetValue(ArrayType v, unsigned int idx = 0)
  {
    assert(idx < GetNumberOfValues());
    Internal[idx] = std::move(v);
  }

  void
  SetValues(const ArrayType * array, unsigned int numel = VMType)
  {
    assert(array && numel && numel == GetNumberOfValues());
    std::copy(array, array + numel, Internal);
  }

  const ArrayType *
  GetValues() const
  {
    return Internal;
  }

  DataElement
  GetAsDataElement() const
  {
    DataElement        ret(GetTag());
    std::ostringstream os;
    EncodingImplementation<VRToEncoding<TVR>::Mode>::Write(Internal, GetNumberOfValues(), os);
    ret.SetVR(GetVR());
    assert(ret.GetVR() != VR::SQ);
    if (static_cast<VR::VRType>(VRToEncoding<TVR>::Mode) == VR::VRASCII)
    {
      if (GetVR() != VR::UI)
      {
        if (os.str().size() % 2)
        {
          os << ' ';
        }
      }
    }
    VL::Type osStrSize = static_cast<VL::Type>(os.str().size());
    ret.SetByteValue(os.str().c_str(), osStrSize);
    return ret;
  }

  void
  SetFromDataElement(const DataElement & de)
  {
    assert(GetTag() == de.GetTag() || GetTag().GetGroup() == 0x6000 || GetTag().GetGroup() == 0x5000); // ?
    assert(GetVR() != VR::INVALID);
    // In case of VR::INVALID cannot use the & operator
    assert(GetVR().Compatible(de.GetVR()) || de.GetVR() == VR::INVALID);
    if (de.IsEmpty())
      return;
    const ByteValue * bv = de.GetByteValue();
#ifdef MDCM_WORDS_BIGENDIAN
    if (de.GetVR() == VR::UN)
#else
    if (de.GetVR() == VR::UN || de.GetVR() == VR::INVALID)
#endif
    {
      SetByteValue(bv);
    }
    else
    {
      SetByteValueNoSwap(bv);
    }
  }

  void
  Set(const DataSet & ds)
  {
    SetFromDataElement(ds.GetDataElement(GetTag()));
  }

  void
  SetFromDataSet(const DataSet & ds)
  {
    const DataElement & de = ds.GetDataElement(GetTag());
    if (!de.IsEmpty())
    {
      SetFromDataElement(de);
    }
  }

protected:
  void
  SetByteValueNoSwap(const ByteValue * bv)
  {
    if (!bv)
      return;
    assert(bv->GetPointer() && bv->GetLength());
    std::stringstream ss;
    std::string       s = std::string(bv->GetPointer(), bv->GetLength());
    ss.str(s);
    EncodingImplementation<VRToEncoding<TVR>::Mode>::ReadNoSwap(Internal, GetNumberOfValues(), ss);
  }

  void
  SetByteValue(const ByteValue * bv)
  {
    if (!bv)
      return;
    assert(bv->GetPointer() && bv->GetLength());
    std::stringstream ss;
    std::string       s = std::string(bv->GetPointer(), bv->GetLength());
    ss.str(s);
    EncodingImplementation<VRToEncoding<TVR>::Mode>::Read(Internal, GetNumberOfValues(), ss);
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM1>
{
public:
  typedef typename VRToType<TVR>::Type ArrayType;

  enum
  {
    VMType = VMToLength<VM::VM1>::Length
  };

  ArrayType Internal;

  static_assert(static_cast<bool>(static_cast<VR::VRType>(TVR) & static_cast<VR::VRType>(TagToType<Group, Element>::VRType)), "");

  static Tag
  GetTag()
  {
    return Tag(Group, Element);
  }

  static VR
  GetVR()
  {
    return static_cast<VR::VRType>(TVR);
  }

  static VM
  GetVM()
  {
    return VM::VM1;
  }

  // The following two methods make sense only for public elements,
  // if the template is intanciated with private element the VR/VM
  // are defaulted to allow everything (VR::VRALL and VM::VM1_n),
  // see mdcmTagToType.h default template.

  static VR
  GetDictVR()
  {
    return static_cast<VR::VRType>(TagToType<Group, Element>::VRType);
  }

  static VM
  GetDictVM()
  {
    return static_cast<VM::VMType>(TagToType<Group, Element>::VMType);
  }

  unsigned int
  GetNumberOfValues() const
  {
    return VMToLength<VM::VM1>::Length;
  }

  void
  Print(std::ostream & os) const
  {
    os << GetTag() << ' '
       << TagToType<Group, Element>::GetVRString() << ' '
       << TagToType<Group, Element>::GetVMString() << ' '
       << Internal;
  }

  bool
  operator==(const Attribute & att) const
  {
    return std::equal(&Internal, &Internal + GetNumberOfValues(), att.GetValues());
  }

  bool
  operator!=(const Attribute & att) const
  {
    return !std::equal(&Internal, &Internal + GetNumberOfValues(), att.GetValues());
  }

  bool
  operator<(const Attribute & att) const
  {
    return std::lexicographical_compare(
      &Internal, &Internal + GetNumberOfValues(), att.GetValues(), att.GetValues() + att.GetNumberOfValues());
  }

  ArrayType &
  GetValue()
  {
    return Internal;
  }

  const ArrayType &
  GetValue() const
  {
    return Internal;
  }

  void
  SetValue(ArrayType v)
  {
    Internal = std::move(v);
  }

  const ArrayType *
  GetValues() const
  {
    return &Internal;
  }

  DataElement
  GetAsDataElement() const
  {
    DataElement        ret(GetTag());
    std::ostringstream os;
    EncodingImplementation<VRToEncoding<TVR>::Mode>::WriteOne(Internal, 1, os);
    ret.SetVR(GetVR());
    assert(ret.GetVR() != VR::SQ);
    if (static_cast<VR::VRType>(VRToEncoding<TVR>::Mode) == VR::VRASCII)
    {
      if (GetVR() != VR::UI)
      {
        if (os.str().size() % 2)
        {
          os << " ";
        }
      }
    }
    VL::Type osStrSize = static_cast<VL::Type>(os.str().size());
    ret.SetByteValue(os.str().c_str(), osStrSize);
    return ret;
  }

  void
  SetFromDataElement(const DataElement & de)
  {
    assert(GetTag() == de.GetTag() || GetTag().GetGroup() == 0x6000 || GetTag().GetGroup() == 0x5000); // ?
    assert(GetVR() != VR::INVALID);
    assert(GetVR().Compatible(de.GetVR()) || de.GetVR() == VR::INVALID);
    if (de.IsEmpty())
      return;
    const ByteValue * bv = de.GetByteValue();
#ifdef MDCM_WORDS_BIGENDIAN
    if (de.GetVR() == VR::UN)
#else
    if (de.GetVR() == VR::UN || de.GetVR() == VR::INVALID)
#endif
    {
      SetByteValue(bv);
    }
    else
    {
      SetByteValueNoSwap(bv);
    }
  }

  void
  Set(const DataSet & ds)
  {
    SetFromDataElement(ds.GetDataElement(GetTag()));
  }

  void
  SetFromDataSet(const DataSet & ds)
  {
    const DataElement & de = ds.GetDataElement(GetTag());
    if (!de.IsEmpty())
    {
      SetFromDataElement(de);
    }
  }

protected:
  void
  SetByteValueNoSwap(const ByteValue * bv)
  {
    if (!bv)
      return;
    assert(bv->GetPointer() && bv->GetLength());
    std::stringstream ss;
    std::string       s = std::string(bv->GetPointer(), bv->GetLength());
    ss.str(s);
    EncodingImplementation<VRToEncoding<TVR>::Mode>::ReadNoSwapOne(Internal, 1, ss);
  }

  void
  SetByteValue(const ByteValue * bv)
  {
    if (!bv)
      return;
    assert(bv->GetPointer() && bv->GetLength());
    std::stringstream ss;
    std::string       s = std::string(bv->GetPointer(), bv->GetLength());
    ss.str(s);
    EncodingImplementation<VRToEncoding<TVR>::Mode>::ReadOne(Internal, 1, ss);
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  Attribute() = default;

  ~Attribute()
  {
    if (Own)
    {
      delete[] Internal;
    }
    Internal = nullptr;
  }

  typedef typename VRToType<TVR>::Type ArrayType;

  static_assert(static_cast<bool>(static_cast<VR::VRType>(TVR) & static_cast<VR::VRType>(TagToType<Group, Element>::VRType)), "");

  static Tag
  GetTag()
  {
    return Tag(Group, Element);
  }

  static VR
  GetVR()
  {
    return static_cast<VR::VRType>(TVR);
  }

  static VM
  GetVM()
  {
    return VM::VM1_n;
  }

  static VR
  GetDictVR()
  {
    return static_cast<VR::VRType>(TagToType<Group, Element>::VRType);
  }

  static VM
  GetDictVM()
  {
    return GetVM();
  }

  unsigned int
  GetNumberOfValues() const
  {
    return Length;
  }

  void
  SetNumberOfValues(unsigned int n)
  {
    SetValues(nullptr, n, true);
  }

  const ArrayType *
  GetValues() const
  {
    return Internal;
  }

  void
  Print(std::ostream & os) const
  {
    os << GetTag() << ' ' << GetVR() << ' ' << GetVM() << ' ';
    if (Length > 0)
      os << Internal[0];
    if (Length > 1)
    {
      for (unsigned int i = 1; i < GetNumberOfValues(); ++i)
        os << ',' << Internal[i];
    }
  }

  ArrayType &
  GetValue(unsigned int idx = 0)
  {
    assert(idx < GetNumberOfValues());
    return Internal[idx];
  }

  ArrayType & operator[](unsigned int idx) { return GetValue(idx); }

  const ArrayType &
  GetValue(unsigned int idx = 0) const
  {
    assert(idx < GetNumberOfValues());
    return Internal[idx];
  }

  const ArrayType & operator[](unsigned int idx) const { return GetValue(idx); }

  void
  SetValue(unsigned int idx, ArrayType v)
  {
    assert(idx < GetNumberOfValues());
    Internal[idx] = std::move(v);
  }

  void
  SetValue(ArrayType v)
  {
    SetValue(0, std::move(v));
  }

  void
  SetValues(const ArrayType * array, unsigned int numel, bool own = false)
  {
    if (Internal)
    {
      if (Own)
      {
        delete[] Internal;
      }
      Internal = nullptr;
    }
    Own = own;
    Length = numel;
    if (own)
    {
      try
      {
        Internal = new ArrayType[numel];
      }
      catch (const std::bad_alloc &)
      {
        return;
      }
      if (array && numel)
      {
        std::copy(array, array + numel, Internal);
      }
    }
    else
    {
      Internal = const_cast<ArrayType *>(array);
    }
    assert(numel == GetNumberOfValues());
  }

  DataElement
  GetAsDataElement() const
  {
    DataElement        ret(GetTag());
    std::ostringstream os;
    if (Internal)
    {
      EncodingImplementation<VRToEncoding<TVR>::Mode>::Write(Internal, GetNumberOfValues(), os);
      if (static_cast<VR::VRType>(VRToEncoding<TVR>::Mode) == VR::VRASCII)
      {
        if (GetVR() != VR::UI)
        {
          if (os.str().size() % 2)
          {
            os << " ";
          }
        }
      }
    }
    ret.SetVR(GetVR());
    assert(ret.GetVR() != VR::SQ);
    VL::Type osStrSize = static_cast<VL::Type>(os.str().size());
    ret.SetByteValue(os.str().c_str(), osStrSize);
    return ret;
  }

  void
  SetFromDataElement(const DataElement & de)
  {
    assert(GetTag() == de.GetTag() || GetTag().GetGroup() == 0x6000 || GetTag().GetGroup() == 0x5000); // ?
    assert(GetVR().Compatible(de.GetVR()));
    assert(!de.IsEmpty());
    const ByteValue * bv = de.GetByteValue();
    SetByteValue(bv);
  }

  void
  Set(const DataSet & ds)
  {
    SetFromDataElement(ds.GetDataElement(GetTag()));
  }

  void
  SetFromDataSet(const DataSet & ds)
  {
    const DataElement & de = ds.GetDataElement(GetTag());
    if (!de.IsEmpty())
    {
      SetFromDataElement(de);
    }
  }

protected:
  void
  SetByteValue(const ByteValue * bv)
  {
    if (!bv)
      return;
    std::stringstream ss;
    std::string       s = std::string(bv->GetPointer(), bv->GetLength());
    Length = bv->GetLength(); // overallocation
    ss.str(s);
    ArrayType * internal;
    try
    {
      internal = new ArrayType[static_cast<VL::Type>(bv->GetLength())]; // overallocation
    }
    catch (const std::bad_alloc &)
    {
      return;
    }
    // update length
    EncodingImplementation<VRToEncoding<TVR>::Mode>::ReadComputeLength(internal, Length, ss);
    SetValues(internal, Length, true);
    delete[] internal;
  }

private:
  ArrayType *  Internal{};
  unsigned int Length{};
  bool         Own{true};
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM1_2> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM1_2;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM1_3> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM1_3;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM1_4> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM1_4;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM1_5> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM1_5;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM1_8> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM1_8;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM1_32> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM1_32;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM1_99> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM1_99;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM2_4> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM2_4;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM3_4> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM3_4;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM4_5> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM4_5;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM2_2n> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  static VM
  GetVM()
  {
    return VM::VM2_2n;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM2_n> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  VM
  GetVM() const
  {
    return VM::VM2_n;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM3_3n> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  static VM
  GetVM()
  {
    return VM::VM3_3n;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM3_n> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  static VM
  GetVM()
  {
    return VM::VM3_n;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM4_4n> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  static VM
  GetVM()
  {
    return VM::VM4_4n;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM6_6n> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  static VM
  GetVM()
  {
    return VM::VM6_6n;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM6_n> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  static VM
  GetVM()
  {
    return VM::VM6_n;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM7_7n> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  static VM
  GetVM()
  {
    return VM::VM7_7n;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM30_30n> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  static VM
  GetVM()
  {
    return VM::VM30_30n;
  }
};

template <uint16_t Group, uint16_t Element, long long TVR>
class Attribute<Group, Element, TVR, VM::VM47_47n> : public Attribute<Group, Element, TVR, VM::VM1_n>
{
public:
  static VM
  GetVM()
  {
    return VM::VM30_30n;
  }
};

} // namespace mdcm

#endif // MDCMATTRIBUTE_H
