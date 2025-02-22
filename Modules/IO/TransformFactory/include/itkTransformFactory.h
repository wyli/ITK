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
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/
#ifndef itkTransformFactory_h
#define itkTransformFactory_h

#include "itkTransformFactoryBase.h"

namespace itk
{
/** \class TransformFactory
 * \brief Create instances of Transforms
 * \ingroup ITKTransformFactory
 *
 * /sphinx
 * /sphinxexample{IO/TransformFactory/RegisterTransformWithTransformFactory,Register Transform With Transform Factory}
 * /endsphinx
 */

template <typename T>
class TransformFactory : public TransformFactoryBase
{
public:
  static void
  RegisterTransform()
  {
    auto t = T::New();

    TransformFactoryBase::Pointer f = TransformFactoryBase::GetFactory();

    f->RegisterTransform(t->GetTransformTypeAsString().c_str(),
                         t->GetTransformTypeAsString().c_str(),
                         t->GetTransformTypeAsString().c_str(),
                         1,
                         CreateObjectFunction<T>::New());
  }
};
} // end namespace itk

#endif
