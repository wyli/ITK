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
#ifndef itkAbsImageAdaptor_h
#define itkAbsImageAdaptor_h

#include "itkImageAdaptor.h"

namespace itk
{
namespace Accessor
{
/**
 *\class AbsPixelAccessor
 * \brief Give access to the itk::Math::abs() function of a value
 *
 * AbsPixelAccessor is templated over an internal type and an
 * external type representation. This class cast the input
 * applies the function to it and cast the result according
 * to the types defined as template parameters
 *
 * \ingroup ImageAdaptors
 * \ingroup ITKImageAdaptors
 */
template <typename TInternalType, typename TExternalType>
class AbsPixelAccessor
{
public:
  /** External type alias. It defines the external aspect
   * that this class will exhibit. */
  using ExternalType = TExternalType;

  /** Internal type alias. It defines the internal real
   * representation of data. */
  using InternalType = TInternalType;

  static inline void
  Set(TInternalType & output, const TExternalType & input)
  {
    output = (TInternalType)((input > NumericTraits<TExternalType>::ZeroValue()) ? input : -input);
  }

  static inline TExternalType
  Get(const TInternalType & input)
  {
    return (TExternalType)((input > NumericTraits<TInternalType>::ZeroValue()) ? input : -input);
  }
};
} // end namespace Accessor

/**
 *\class AbsImageAdaptor
 * \brief Presents an image as being composed of the itk::Math::abs() of its pixels
 *
 * Additional casting is performed according to the input and output image
 * types following C++ default casting rules.
 *
 * \ingroup ImageAdaptors
 * \ingroup ITKImageAdaptors
 */
template <typename TImage, typename TOutputPixelType>
class AbsImageAdaptor
  : public ImageAdaptor<TImage, Accessor::AbsPixelAccessor<typename TImage::PixelType, TOutputPixelType>>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(AbsImageAdaptor);

  /** Standard class type aliases. */
  using Self = AbsImageAdaptor;
  using Superclass = ImageAdaptor<TImage, Accessor::AbsPixelAccessor<typename TImage::PixelType, TOutputPixelType>>;

  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AbsImageAdaptor, ImageAdaptor);

protected:
  AbsImageAdaptor() = default;
  ~AbsImageAdaptor() override = default;
};
} // end namespace itk

#endif
