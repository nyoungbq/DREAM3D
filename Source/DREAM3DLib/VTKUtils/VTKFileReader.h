/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef VTKFILEREADER_H_
#define VTKFILEREADER_H_

#include <fstream>


#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/Common/FileReader.h"

/**
 * @class VTKFileReader VTKFileReader.h PathToHeader/VTKFileReader.h
 * @brief This class holds some basic methods to read various items from a legacy
 * VTK file.
 *
 * <b>Legacy VTK File Header </b>
 * @code
   1: # vtk DataFile Version 2.0
   2: NRL TiBeta Stack
   3: BINARY
   4: DATASET STRUCTURED_POINTS
   5: DIMENSIONS 1670 770 201
   6: ORIGIN 0.00 0.00 0.00
   7: SPACING 0.665 0.665 1.48
   8: POINT_DATA 258465900
   9:
   10: SCALARS GrainID int 1
   11: LOOKUP_TABLE default

   OR

   1: # vtk DataFile Version 2.0
   2: data set from FFT2dx_GB
   3: ASCII
   4: DATASET STRUCTURED_POINTS
   5: DIMENSIONS 189 201 100
   6: ORIGIN 0.0 0.0 0.0
   7: SPACING 0.25 0.25 0.25
   8: POINT_DATA 3798900
   9:
   10: SCALARS GrainID int 1
   11: LOOKUP_TABLE default
 * @endcode
 *
 *
 *
 * @author Michael A. Jackson for BlueQuartz Software
 * @date May 19, 2011
 * @version 1.0
 */
class DREAM3DLib_EXPORT VTKFileReader : public FileReader
{
  public:
    DREAM3D_SHARED_POINTERS(VTKFileReader)
    DREAM3D_TYPE_MACRO_SUPER(VTKFileReader, FileReader)
    DREAM3D_STATIC_NEW_MACRO(VTKFileReader)

    virtual ~VTKFileReader();

   // DREAM3D_INSTANCE_STRING_PROPERTY(InputFileName)
    DREAM3D_INSTANCE_STRING_PROPERTY(Comment)
    DREAM3D_INSTANCE_STRING_PROPERTY(DatasetType)
    DREAM3D_INSTANCE_PROPERTY(bool, FileIsBinary)

    /**
     * @brief Reads the VTK header and sets the values that are described in the header
     * @return Error Condition. Negative is Error.
     */
    int readHeader();

    /**
     * @brief This method should be re-implemented in a subclass
     */
    virtual int readFile() {
      return -1;
    }

    /**
     * @brief Parses the byte size from a data set declaration line
     * @param text
     * @return
     */
    size_t parseByteSize(char text[256]);

    /**
     *
     */
    int ignoreData(std::ifstream &in, int byteSize, char* type, int xDim, int yDim, int zDim);

    /**
     *
     */
    template<typename T>
    int skipVolume(std::ifstream &inStream, int byteSize, int xDim, int yDim, int zDim, T &diff)
    {
      int err = 0;
      size_t totalSize = xDim * yDim * zDim;
      if (getFileIsBinary() == true)
      {
        T* buffer = new T[totalSize];
        // Read all the xpoints in one shot into a buffer
        inStream.read(reinterpret_cast<char* > (buffer), (totalSize * sizeof(T)));
        if(inStream.gcount() != static_cast<std::streamsize>(totalSize * sizeof(T)))
        {
          std::cout << " ERROR READING BINARY FILE. Bytes read was not the same as func->xDim *. " << byteSize << "." << inStream.gcount()
              << " vs " << (totalSize * sizeof(T)) << std::endl;
          return -1;
        }
        if (totalSize > 1) {
          diff = buffer[totalSize-1] - buffer[totalSize-2];
        }
        else
        {
          diff = buffer[totalSize];
        }
        delete buffer;
      }
      else
      {
        T tmp;
        T t2;
        for (size_t x = 0; x < totalSize; ++x)
        {
          if(x == 1)
          {
            t2 = tmp;
          }
          inStream >> tmp;
          if(x == 1)
          {
            diff = tmp - t2;
          }
        }
      }
      return err;
    }

    template<typename T>
    int skipVolume(std::ifstream &inStream, int byteSize, int xDim, int yDim, int zDim)
    {
      int err = 0;
      if(getFileIsBinary() == true)
      {
        size_t totalSize = xDim * yDim * zDim;
        T* buffer = new T[totalSize];
        // Read all the xpoints in one shot into a buffer
        inStream.read(reinterpret_cast<char*>(buffer), (totalSize * sizeof(T)));
        if(inStream.gcount() != static_cast<std::streamsize>(totalSize * sizeof(T)))
        {
          std::cout << " ERROR READING BINARY FILE. Bytes read was not the same as func->xDim *. " << byteSize << "." << inStream.gcount() << " vs "
              << (totalSize * sizeof(T)) << std::endl;
          return -1;
        }
        delete buffer;
      }
      else
      {
        T tmp;
        for (int z = 0; z < zDim; ++z)
        {
          for (int y = 0; y < yDim; ++y)
          {
            for (int x = 0; x < xDim; ++x)
            {
              inStream >> tmp;
            }
          }
        }
      }
      return err;
    }


  protected:
    VTKFileReader();


  private:
    VTKFileReader(const VTKFileReader&); // Copy Constructor Not Implemented
    void operator=(const VTKFileReader&); // Operator '=' Not Implemented
};

#endif /* VTKFILEREADER_H_ */
