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
#include "mdcmCurve.h"
#include "mdcmByteValue.h"
#include "mdcmDataElement.h"
#include "mdcmDataSet.h"
#include "mdcmAttribute.h"
#include <vector>

namespace mdcm
{

unsigned int
Curve::GetNumberOfCurves(const DataSet & ds)
{
  Tag          overlay(0x5000, 0x0000); // first possible overlay
  bool         finished = false;
  unsigned int numoverlays = 0;
  while (!finished)
  {
    const DataElement & de = ds.FindNextDataElement(overlay);
    if (de.GetTag().GetGroup() > 0x50FF) // last possible curve
    {
      finished = true;
    }
    else if (de.GetTag().IsPrivate())
    {
      overlay.SetGroup(static_cast<uint16_t>(de.GetTag().GetGroup() + 1));
      overlay.SetElement(0);
    }
    else
    {
      const DataElement & overlaydata = ds.GetDataElement(Tag(de.GetTag().GetGroup(), 0x3000));
      if (!overlaydata.IsEmpty())
      {
        ++numoverlays;
      }
      // store found tag in overlay
      overlay = de.GetTag();
      // move on to the next possible one
      overlay.SetGroup(static_cast<uint16_t>(overlay.GetGroup() + 2));
      // reset to element 0x0 just in case
      overlay.SetElement(0);
    }
  }
  return numoverlays;
}

void
Curve::Update(const DataElement & de)
{
  const ByteValue * bv = de.GetByteValue();
  if (!bv)
    return;
  std::string s(bv->GetPointer(), bv->GetLength());
  if (!GetGroup())
  {
    SetGroup(de.GetTag().GetGroup());
  }
  else
  {
    assert(GetGroup() == de.GetTag().GetGroup());
  }
  if (de.GetTag().GetElement() == 0x0000) // CurveGroupLength
  {
    ;
  }
  else if (de.GetTag().GetElement() == 0x0005) // CurveDimensions
  {
    Attribute<0x5000, 0x0005> at;
    at.SetFromDataElement(de);
    SetDimensions(at.GetValue());
  }
  else if (de.GetTag().GetElement() == 0x0010) // NumberOfPoints
  {
    Attribute<0x5000, 0x0010> at;
    at.SetFromDataElement(de);
    SetNumberOfPoints(at.GetValue());
  }
  else if (de.GetTag().GetElement() == 0x0020) // TypeOfData
  {
    SetTypeOfData(s.c_str());
  }
  else if (de.GetTag().GetElement() == 0x0022) // CurveDescription
  {
    SetCurveDescription(s.c_str());
  }
  else if (de.GetTag().GetElement() == 0x0030) // AxisUnits
  {
    mdcmWarningMacro("TODO");
  }
  else if (de.GetTag().GetElement() == 0x0040) // Axis Labels
  {
    mdcmWarningMacro("TODO");
  }
  else if (de.GetTag().GetElement() == 0x0103) // DataValueRepresentation
  {
    Attribute<0x5000, 0x0103> at;
    at.SetFromDataElement(de);
    SetDataValueRepresentation(at.GetValue());
  }
  else if (de.GetTag().GetElement() == 0x0104) // Minimum Coordinate Value
  {
    mdcmWarningMacro("TODO");
  }
  else if (de.GetTag().GetElement() == 0x0105) // Maximum Coordinate Value
  {
    mdcmWarningMacro("TODO");
  }
  else if (de.GetTag().GetElement() == 0x0106) // Curve Range
  {
    mdcmWarningMacro("TODO");
  }
  else if (de.GetTag().GetElement() == 0x0110) // CurveDataDescriptor
  {
    Attribute<0x5000, 0x0110> at;
    at.SetFromDataElement(de);
    SetCurveDataDescriptor(at.GetValues(), at.GetNumberOfValues());
  }
  else if (de.GetTag().GetElement() == 0x0112) // CoordinateStartValue
  {
    Attribute<0x5000, 0x0112> at;
    at.SetFromDataElement(de);
    SetCoordinateStartValue(at.GetValue());
  }
  else if (de.GetTag().GetElement() == 0x0114) // CoordinateStepValue
  {
    Attribute<0x5000, 0x0114> at;
    at.SetFromDataElement(de);
    SetCoordinateStepValue(at.GetValue());
  }
  else if (de.GetTag().GetElement() == 0x2500) // CurveLabel
  {
    mdcmWarningMacro("TODO");
  }
  else if (de.GetTag().GetElement() == 0x2600) // Referenced Overlay Sequence
  {
    mdcmWarningMacro("TODO");
  }
  else if (de.GetTag().GetElement() == 0x2610) // Referenced Overlay Group
  {
    mdcmWarningMacro("TODO");
  }
  else if (de.GetTag().GetElement() == 0x3000) // CurveData
  {
    SetCurve(bv->GetPointer(), bv->GetLength());
  }
  else
  {
    assert(0 && "should not happen: Unknown curve tag");
  }
}

void
Curve::SetGroup(unsigned short group)
{
  Internal.Group = group;
}

unsigned short
Curve::GetGroup() const
{
  return Internal.Group;
}

void
Curve::SetDimensions(unsigned short dimensions)
{
  Internal.Dimensions = dimensions;
}

unsigned short
Curve::GetDimensions() const
{
  return Internal.Dimensions;
}

void
Curve::SetNumberOfPoints(unsigned short numberofpoints)
{
  Internal.NumberOfPoints = numberofpoints;
}

unsigned short
Curve::GetNumberOfPoints() const
{
  return Internal.NumberOfPoints;
}

void
Curve::SetTypeOfData(const char * typeofdata)
{
  if (typeofdata)
    Internal.TypeOfData = typeofdata;
}

const char *
Curve::GetTypeOfData() const
{
  return Internal.TypeOfData.c_str();
}

// See PS 3.3 - 2004 - C.10.2.1.1 Type of data
static const char * const TypeOfDataDescription[][2] = { { "TAC", "time activity curve" },
                                                         { "PROF", "image profile" },
                                                         { "HIST", "histogram" },
                                                         { "ROI", "polygraphic region of interest" },
                                                         { "TABL", "table of values" },
                                                         { "FILT", "filter kernel" },
                                                         { "POLY", "poly line" },
                                                         { "ECG", "ecg data" },
                                                         { "PRESSURE", "pressure data" },
                                                         { "FLOW", "flow data" },
                                                         { "PHYSIO", "physio data" },
                                                         { "RESP", "Respiration trace" },
                                                         { nullptr, nullptr } };

const char *
Curve::GetTypeOfDataDescription() const
{
  typedef const char * const(*TypeOfDataDescriptionType)[2];
  TypeOfDataDescriptionType t = TypeOfDataDescription;
  int                       i = 0;
  const char *              p = t[i][0];
  while (p)
  {
    if (Internal.TypeOfData == p)
    {
      break;
    }
    ++i;
    p = t[i][0];
  }
  return t[i][1];
}

void
Curve::SetCurveDescription(const char * curvedescription)
{
  if (curvedescription)
    Internal.CurveDescription = curvedescription;
}

void
Curve::SetDataValueRepresentation(unsigned short datavaluerepresentation)
{
  Internal.DataValueRepresentation = datavaluerepresentation;
}

unsigned short
Curve::GetDataValueRepresentation() const
{
  return Internal.DataValueRepresentation;
}

void
Curve::SetCurveDataDescriptor(const uint16_t * values, size_t num)
{
  Internal.CurveDataDescriptor = std::vector<uint16_t>(values, values + num);
}

const std::vector<unsigned short> &
Curve::GetCurveDataDescriptor() const
{
  return Internal.CurveDataDescriptor;
}

void
Curve::SetCoordinateStartValue(unsigned short v)
{
  Internal.CoordinateStartValue = v;
}

void
Curve::SetCoordinateStepValue(unsigned short v)
{
  Internal.CoordinateStepValue = v;
}

bool
Curve::IsEmpty() const
{
  return Internal.Data.empty();
}

void
Curve::SetCurve(const char * array, unsigned int length)
{
  if (!array || length == 0)
    return;
  Internal.Data.resize(length);
  std::copy(array, array + length, Internal.Data.begin());
}

void
Curve::Decode(std::istream &, std::ostream &)
{
  assert(0);
}

/*
PS 3.3 - 2004
C.10.2.1.2 Data value representation
0000H = unsigned short (US)
0001H = signed short (SS)
0002H = floating point single (FL)
0003H = floating point double (FD)
0004H = signed long (SL)
*/
inline size_t
getsizeofrep(unsigned short dr)
{
  size_t val = 0;
  switch (dr)
  {
    case 0:
      val = sizeof(uint16_t);
      break;
    case 1:
      val = sizeof(int16_t);
      break;
    case 2:
      val = sizeof(float);
      break;
    case 3:
      val = sizeof(double);
      break;
    case 4:
      val = sizeof(int32_t);
      break;
    default:
      break;
  }
  return val;
}

void
Curve::GetAsPoints(float * array) const
{
  assert(getsizeofrep(Internal.DataValueRepresentation));
  if (Internal.CurveDataDescriptor.empty())
  {
    assert(Internal.Data.size() ==
           static_cast<uint32_t>(Internal.NumberOfPoints) * Internal.Dimensions * getsizeofrep(Internal.DataValueRepresentation));
  }
  else
  {
    assert(Internal.Data.size() ==
           static_cast<uint32_t>(Internal.NumberOfPoints) * 1 * getsizeofrep(Internal.DataValueRepresentation));
  }
  assert(Internal.Dimensions == 1 || Internal.Dimensions == 2);
  const int mult = Internal.Dimensions;
  int       genidx = -1;
  if (!Internal.CurveDataDescriptor.empty())
  {
    assert(Internal.CurveDataDescriptor.size() == Internal.Dimensions);
    assert(Internal.CurveDataDescriptor.size() == 2); // FIXME
    if (Internal.CurveDataDescriptor[0] == 0)
    {
      assert(Internal.CurveDataDescriptor[1] == 1);
      genidx = 0;
    }
    else if (Internal.CurveDataDescriptor[1] == 0)
    {
      assert(Internal.CurveDataDescriptor[0] == 1);
      genidx = 1;
    }
    else
    {
      assert(0 && "TODO");
    }
  }
  const char * beg = &Internal.Data[0];
  const char * end = beg + Internal.Data.size();
  if (genidx == -1)
  {
    assert(end == beg + 2 * Internal.NumberOfPoints);
    (void)beg;
    (void)end;
  }
  else
  {
    assert(end == beg + mult * Internal.NumberOfPoints);
    (void)beg;
    (void)end;
  }
  const void * vp = static_cast<const void*>(Internal.Data.data());
  if (Internal.DataValueRepresentation == 0)
  {
    // PS 3.3 - 2004
    // C.10.2.1.5 Curve data descriptor, coordinate start value, coordinate step value
    const uint16_t * p = static_cast<const uint16_t *>(vp);
    // X
    if (genidx == 0)
    {
      for (int i = 0; i < Internal.NumberOfPoints; ++i)
        array[3 * i + 0] = static_cast<float>(ComputeValueFromStartAndStep(i));
    }
    else
    {
      for (int i = 0; i < Internal.NumberOfPoints; ++i)
        array[3 * i + 0] = p[i + 0];
    }
    // Y
    if (genidx == 1)
    {
      for (int i = 0; i < Internal.NumberOfPoints; ++i)
        array[3 * i + 1] = static_cast<float>(ComputeValueFromStartAndStep(i));
    }
    else
    {
      if (mult == 2 && genidx == -1)
      {
        for (int i = 0; i < Internal.NumberOfPoints; ++i)
          array[3 * i + 1] = p[i + 1];
      }
      else if (mult == 2 && genidx == 0)
      {
        for (int i = 0; i < Internal.NumberOfPoints; ++i)
          array[3 * i + 1] = p[i + 0];
      }
      else
      {
        for (int i = 0; i < Internal.NumberOfPoints; ++i)
          array[3 * i + 1] = 0;
      }
    }
    // Z
    for (int i = 0; i < Internal.NumberOfPoints; ++i)
      array[3 * i + 2] = 0;
  }
  else if (Internal.DataValueRepresentation == 1)
  {
    const int16_t * p = static_cast<const int16_t *>(vp);
    for (int i = 0; i < Internal.NumberOfPoints; ++i)
    {
      array[3 * i + 0] = p[mult * i + 0];
      if (mult > 1)
        array[3 * i + 1] = p[mult * i + 1];
      else
        array[3 * i + 1] = 0;
      array[3 * i + 2] = 0;
    }
  }
  else if (Internal.DataValueRepresentation == 2)
  {
    const float * p = static_cast<const float *>(vp);
    for (int i = 0; i < Internal.NumberOfPoints; ++i)
    {
      array[3 * i + 0] = p[mult * i + 0];
      if (mult > 1)
        array[3 * i + 1] = p[mult * i + 1];
      else
        array[3 * i + 1] = 0;
      array[3 * i + 2] = 0;
    }
  }
  else if (Internal.DataValueRepresentation == 3)
  {
    const double * p = static_cast<const double *>(vp);
    for (int i = 0; i < Internal.NumberOfPoints; ++i)
    {
      array[3 * i + 0] = static_cast<float>(p[mult * i + 0]);
      if (mult > 1)
        array[3 * i + 1] = static_cast<float>(p[mult * i + 1]);
      else
        array[3 * i + 1] = 0;
      array[3 * i + 2] = 0;
    }
  }
  else if (Internal.DataValueRepresentation == 4)
  {
    const int32_t * p = static_cast<const int32_t *>(vp);
    for (int i = 0; i < Internal.NumberOfPoints; ++i)
    {
      array[3 * i + 0] = static_cast<float>(p[mult * i + 0]);
      if (mult > 1)
        array[3 * i + 1] = static_cast<float>(p[mult * i + 1]);
      else
        array[3 * i + 1] = 0;
      array[3 * i + 2] = 0;
    }
  }
  else
  {
    assert(0);
  }
}

void
Curve::Print(std::ostream & os) const
{
  os << "Group           0x" << std::hex << Internal.Group << std::dec
     << "\nDimensions              :" << Internal.Dimensions
     << "\nNumberOfPoints          :" << Internal.NumberOfPoints
     << "\nTypeOfData              :" << Internal.TypeOfData
     << "\nCurveDescription        :" << Internal.CurveDescription
     << "\nDataValueRepresentation :" << Internal.DataValueRepresentation << '\n';
  const void * dp = static_cast<const void*>(Internal.Data.data());
  const unsigned short * p = static_cast<const unsigned short *>(dp);
  for (int i = 0; i < Internal.NumberOfPoints; i += 2)
  {
    os << p[i] << ',' << p[i + 1] << '\n';
  }
  os << std::endl;
}

/*
C.10.2.1.5 Curve data descriptor, coordinate start value, coordinate step value
The Curve Data for dimension(s) containing evenly distributed data can be eliminated by using a
method that defines the Coordinate Start Value and Coordinate Step Value (interval). The one
dimensional data list is then calculated rather than being enumerated.
For the Curve Data Descriptor (50xx,0110) an Enumerated Value describing how each
component of the N-tuple curve is described, either by points or interval spacing. One value for
each dimension. Where:
0000H = Dimension component described using interval spacing
0001H = Dimension component described using values
Using interval spacing:
Dimension component(s) described by interval spacing use Attributes of Coordinate Start Value
(50xx,0112), Coordinate Step Value (50xx,0114) and Number of Points (50xx,0010). The 1-
dimensional data list is calculated by using a start point of Coordinate Start Value and adding the
interval (Coordinate Step Value) to obtain each data point until the Number of Points is satisfied.
The data points of this dimension will be absent from Curve Data (50xx,3000).
*/
double
Curve::ComputeValueFromStartAndStep(unsigned int idx) const
{
  assert(!Internal.CurveDataDescriptor.empty());
  const double res = Internal.CoordinateStartValue + Internal.CoordinateStepValue * idx;
  return res;
}

} // namespace mdcm
