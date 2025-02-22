/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkBinaryImageToLevelSetImageAdaptor.h"
#include "itkLevelSetDomainPartitionImage.h"
#include "itkTestingMacros.h"

int
itkLevelSetDomainPartitionImageTest(int, char *[])
{
  constexpr unsigned int Dimension = 2;

  using InputPixelType = unsigned short;
  using InputImageType = itk::Image<InputPixelType, Dimension>;
  using IdentifierType = itk::IdentifierType;

  using DomainPartitionSourceType = itk::LevelSetDomainPartitionImage<InputImageType>;
  using ListImageType = DomainPartitionSourceType::ListImageType;
  using LevelSetDomainRegionVectorType = DomainPartitionSourceType::LevelSetDomainRegionVectorType;

  using ListType = ListImageType::PixelType;
  using ListImageIteratorType = itk::ImageRegionConstIteratorWithIndex<ListImageType>;

  // load binary mask
  InputImageType::SizeType size;
  size.Fill(50);

  InputImageType::PointType origin;
  origin[0] = 0.0;
  origin[1] = 0.0;

  InputImageType::SpacingType spacing;
  spacing[0] = 1.0;
  spacing[1] = 1.0;

  InputImageType::IndexType index;
  index.Fill(0);

  InputImageType::RegionType region;
  region.SetIndex(index);
  region.SetSize(size);

  // Binary initialization
  auto binary = InputImageType::New();
  binary->SetRegions(region);
  binary->SetSpacing(spacing);
  binary->SetOrigin(origin);
  binary->Allocate();
  binary->FillBuffer(itk::NumericTraits<InputPixelType>::ZeroValue());

  IdentifierType numberOfLevelSetFunctions = 2;

  LevelSetDomainRegionVectorType regionVector;
  regionVector.resize(numberOfLevelSetFunctions);
  regionVector[0] = region;
  regionVector[1] = region;

  auto partitionSource = DomainPartitionSourceType::New();
  partitionSource->SetNumberOfLevelSetFunctions(numberOfLevelSetFunctions);

  partitionSource->SetLevelSetDomainRegionVector(regionVector);

  // Exercise exceptions
  ITK_TRY_EXPECT_EXCEPTION(partitionSource->PopulateListDomain());

  partitionSource->SetImage(binary);
  partitionSource->PopulateListDomain();


  bool flag = true;

  ListType                    ll;
  ListImageType::ConstPointer listImage = partitionSource->GetListDomain();
  ListImageIteratorType       It(listImage, listImage->GetLargestPossibleRegion());
  It.GoToBegin();
  while (!It.IsAtEnd())
  {
    ll = It.Get();
    if (ll.size() != 2)
    {
      flag = false;
      break;
    }

    auto it = ll.begin();

    while (it != ll.end())
    {
      if ((*it != 0) && (*it != 1))
      {
        flag = false;
        break;
      }
      ++it;
    }

    ++It;
  }

  if (!flag)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
