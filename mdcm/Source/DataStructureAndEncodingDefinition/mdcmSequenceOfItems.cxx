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

#include "mdcmSequenceOfItems.h"

namespace
{

  static const mdcm::Item empty{};

}

namespace mdcm
{

void
SequenceOfItems::AddItem(const Item & item)
{
  Items.push_back(item);
  if (!SequenceLengthField.IsUndefined())
  {
    assert(0); // TODO
  }
}

Item &
SequenceOfItems::AddNewUndefinedLengthItem()
{
  Item itemToAdd;
  itemToAdd.SetVLToUndefined();
  this->AddItem(itemToAdd);
  return GetItem(this->GetNumberOfItems());
}

void
SequenceOfItems::Clear()
{
  Items.clear();
  assert(SequenceLengthField.IsUndefined());
}

bool
SequenceOfItems::RemoveItemByIndex(const SizeType position)
{
  if (position < 1 || position > Items.size())
  {
    return false;
  }
  Items.erase(Items.begin() + position - 1);
  return true;
}

Item &
SequenceOfItems::GetItem(SizeType position)
{
  if (position < 1 || position > Items.size())
  {
    mdcmAlwaysWarnMacro("SQ: invalid index");
    error_fallback.Clear();
    return error_fallback;
  }
  return Items[position - 1];
}

const Item &
SequenceOfItems::GetItem(SizeType position) const
{
  if (position < 1 || position > Items.size())
  {
    mdcmAlwaysWarnMacro("SQ: invalid index");
    return empty;
  }
  return Items.at(position - 1);
}

void
SequenceOfItems::SetLengthToUndefined()
{
  SequenceLengthField = 0xFFFFFFFF;
}

bool
SequenceOfItems::FindDataElement(const Tag & t) const
{
  for (ConstIterator it = Begin(); it != End(); ++it)
  {
    const Item & item = *it;
    const bool f = item.FindDataElement(t);
    if (f)
      return true;
  }
  return false;
}

} // end namespace mdcm
