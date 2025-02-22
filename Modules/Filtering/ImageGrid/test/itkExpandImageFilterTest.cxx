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

#include <iostream>

#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkExpandImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkStreamingImageFilter.h"
#include "itkMath.h"

// class to produce a linear image pattern
template <int VDimension>
class ImagePattern
{
public:
  using IndexType = itk::Index<VDimension>;

  ImagePattern()
  {
    m_Offset = 0.0;
    for (int j = 0; j < VDimension; ++j)
    {
      m_Coeff[j] = 0.0;
    }
  }

  double
  Evaluate(const IndexType & index)
  {
    double accum = m_Offset;
    for (int j = 0; j < VDimension; ++j)
    {
      accum += m_Coeff[j] * (double)index[j];
    }
    return accum;
  }

  double m_Coeff[VDimension];
  double m_Offset;
};

// The following three classes are used to support callbacks
// on the filter in the pipeline that follows later
class ShowProgressObject
{
public:
  ShowProgressObject(itk::ProcessObject * o) { m_Process = o; }
  void
  ShowProgress()
  {
    std::cout << "Progress " << m_Process->GetProgress() << std::endl;
  }
  itk::ProcessObject::Pointer m_Process;
};


int
itkExpandImageFilterTest(int, char *[])
{
  using PixelType = float;
  enum
  {
    ImageDimension = 2
  };
  using ImageType = itk::Image<PixelType, ImageDimension>;

  bool testPassed = true;


  //=============================================================

  std::cout << "Create the input image pattern." << std::endl;
  ImageType::RegionType region;
  ImageType::SizeType   size = { { 64, 64 } };
  region.SetSize(size);

  auto input = ImageType::New();
  input->SetLargestPossibleRegion(region);
  input->SetBufferedRegion(region);
  input->Allocate();

  int                          j;
  ImagePattern<ImageDimension> pattern;
  pattern.m_Offset = 64;
  for (j = 0; j < ImageDimension; ++j)
  {
    pattern.m_Coeff[j] = 1.0;
  }

  using Iterator = itk::ImageRegionIteratorWithIndex<ImageType>;
  Iterator inIter(input, region);

  while (!inIter.IsAtEnd())
  {
    inIter.Set(pattern.Evaluate(inIter.GetIndex()));
    ++inIter;
  }

  //=============================================================

  std::cout << "Run ExpandImageFilter in standalone mode with progress.";
  std::cout << std::endl;
  using ExpanderType = itk::ExpandImageFilter<ImageType, ImageType>;
  auto expander = ExpanderType::New();

  using InterpolatorType = itk::NearestNeighborInterpolateImageFunction<ImageType, double>;
  auto interpolator = InterpolatorType::New();

  expander->SetInterpolator(interpolator);
  std::cout << "Interpolator: " << expander->GetInterpolator() << std::endl;

  expander->SetExpandFactors(5);

  unsigned int         factors[ImageDimension] = { 2, 3 };
  ImageType::PixelType padValue = 4.0;
  expander->SetInput(input);
  expander->SetExpandFactors(factors);
  // TEST_RMV20100728   expander->SetEdgePaddingValue( padValue );


  ShowProgressObject                                    progressWatch(expander);
  itk::SimpleMemberCommand<ShowProgressObject>::Pointer command;
  command = itk::SimpleMemberCommand<ShowProgressObject>::New();
  command->SetCallbackFunction(&progressWatch, &ShowProgressObject::ShowProgress);
  expander->AddObserver(itk::ProgressEvent(), command);

  expander->Print(std::cout);
  expander->Update();

  //=============================================================

  std::cout << "Checking the output against expected." << std::endl;
  Iterator outIter(expander->GetOutput(), expander->GetOutput()->GetBufferedRegion());

  // compute non-padded output region
  ImageType::RegionType validRegion = expander->GetOutput()->GetLargestPossibleRegion();
  ImageType::SizeType   validSize = validRegion.GetSize();

  validRegion.SetSize(validSize);

  ImageType * expanderOutput = expander->GetOutput();

  while (!outIter.IsAtEnd())
  {
    ImageType::IndexType index = outIter.GetIndex();
    double               value = outIter.Get();

    if (validRegion.IsInside(index))
    {

      ImageType::PointType point;
      expanderOutput->TransformIndexToPhysicalPoint(outIter.GetIndex(), point);
      ImageType::IndexType inputIndex = input->TransformPhysicalPointToIndex(point);
      double               trueValue = pattern.Evaluate(inputIndex);

      if (itk::Math::abs(trueValue - value) > 1e-4)
      {
        testPassed = false;
        std::cout << "Error at Index: " << index << " ";
        std::cout << "Expected: " << trueValue << " ";
        std::cout << "Actual: " << value << std::endl;
      }
    }
    else
    {
      if (itk::Math::NotExactlyEquals(value, padValue))
      {
        testPassed = false;
        std::cout << "Error at Index: " << index << " ";
        std::cout << "Expected: " << padValue << " ";
        std::cout << "Actual: " << value << std::endl;
      }
    }
    ++outIter;
  }

  //=============================================================

  std::cout << "Run ExpandImageFilter with streamer";
  std::cout << std::endl;

  using CasterType = itk::CastImageFilter<ImageType, ImageType>;
  auto caster = CasterType::New();

  caster->SetInput(expander->GetInput());


  auto expander2 = ExpanderType::New();

  expander2->SetInput(caster->GetOutput());
  expander2->SetExpandFactors(expander->GetExpandFactors());
  // TEST_RMV20100728   expander2->SetEdgePaddingValue( expander->GetEdgePaddingValue() );
  expander2->SetInterpolator(interpolator);

  using StreamerType = itk::StreamingImageFilter<ImageType, ImageType>;
  auto streamer = StreamerType::New();
  streamer->SetInput(expander2->GetOutput());
  streamer->SetNumberOfStreamDivisions(3);
  streamer->Update();

  //=============================================================
  std::cout << "Compare standalone and streamed outputs" << std::endl;

  Iterator streamIter(streamer->GetOutput(), streamer->GetOutput()->GetBufferedRegion());

  outIter.GoToBegin();
  streamIter.GoToBegin();

  while (!outIter.IsAtEnd())
  {
    if (itk::Math::NotExactlyEquals(outIter.Get(), streamIter.Get()))
    {
      testPassed = false;
    }
    ++outIter;
    ++streamIter;
  }


  if (!testPassed)
  {
    std::cout << "Test failed." << std::endl;
    return EXIT_FAILURE;
  }

  // Test error handling

  try
  {
    testPassed = false;
    std::cout << "Setting Input to nullptr" << std::endl;
    expander->SetInput(nullptr);
    expander->Update();
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cout << err << std::endl;
    expander->ResetPipeline();
    expander->SetInput(input);
    testPassed = true;
  }

  if (!testPassed)
  {
    std::cout << "Test failed." << std::endl;
    return EXIT_FAILURE;
  }


  try
  {
    testPassed = false;
    std::cout << "Setting Interpolator to nullptr" << std::endl;
    expander->SetInterpolator(nullptr);
    expander->Update();
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cout << err << std::endl;
    expander->ResetPipeline();
    expander->SetInterpolator(interpolator);
    testPassed = true;
  }

  if (!testPassed)
  {
    std::cout << "Test failed." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}
