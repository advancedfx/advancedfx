// You only need to implement IImageData and use it with EasyDecoder.Decode (get one from EasyDecoder.Default), BUT don't forget to handle possible exceptions!
// See Program.cs for example.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace advancedfx.PGL {

    struct Pixel
    {
        /// <summary>
        /// Red value in [0.0f,1.0f].
        /// </summary>
        public float Red;

        /// <summary>
        /// Green value in [0.0f,1.0f].
        /// </summary>
        public float Green;

        /// <summary>
        /// Blue value in [0.0f,1.0f].
        /// </summary>
        public float Blue;
    }

    interface IImageData
    {
        /// <remarks>
        /// Please throw an System.ApplicationException if data is read out of bounds!
        /// </remarks>
        Pixel GetPixel(int x, int y);
    }

    class EasyData
    {
        public EasyData(CameraData cameraData)
        {
            m_CameraData = cameraData;
        }

        public CameraData CameraData
        {
            get
            {
                return m_CameraData;
            }
        }   

        private CameraData m_CameraData;
    }

    class EasyDecoder : IPixelDecoder
    {
        public static EasyDecoder Default()
        {
            return new EasyDecoder(
                90,
                4,
                4,
                ChannelSettings.Default(),
                ChannelSettings.Default(),
                ChannelSettings.Default());
        }

        public EasyDecoder(
            int rectsPerRow,
            int rectWidth,
            int rectHeight,
            ChannelSettings red,
            ChannelSettings green,
            ChannelSettings blue)
        {
            m_RectsPerRow = rectsPerRow;
            m_RectWidth = rectWidth;
            m_RectHeight = rectHeight;

            m_CameraDataDecoder = new CameraDataDecoder(
                new DataDecoder(
                    new BitsDecoder(
                        this, red, green, blue
                    )
                )
            );
        }

        public EasyData Decode(IImageData imageData)
        {
            m_RectsRead = 0;
            m_ImageData = imageData;

            CameraData cameraData = m_CameraDataDecoder.DecodeCameraData();
            m_CameraDataDecoder.Flush();

            return new EasyData(cameraData);
        }

        void IPixelDecoder.Flush()
        {
            // Could uset his to implement a check, if it was properly flushed or not.
        }

        Pixel IPixelDecoder.DecodePixel()
        {
            Pixel result;
            result.Red = 0;
            result.Green = 0;
            result.Blue = 0;

            int rectCol = m_RectsRead % m_RectsPerRow;
            int rectRow = m_RectsRead / m_RectsPerRow;
            int x = rectCol * m_RectWidth;
            int y = rectRow * m_RectHeight;

            for(int j=0; j < m_RectHeight; ++j)
            {
                for(int i=0; i < m_RectWidth; ++i)
                {
                    Pixel imagePixel = m_ImageData.GetPixel(x + i, y + j);

                    result.Red += imagePixel.Red;
                    result.Green += imagePixel.Green;
                    result.Blue += imagePixel.Blue;
                }
            }

            int pixels = m_RectHeight * m_RectWidth;

            if ( 0 < pixels)
            {
                result.Red /= pixels;
                result.Green /= pixels;
                result.Blue /= pixels;
            }

            ++m_RectsRead;

            return result;
        }

        private CameraDataDecoder m_CameraDataDecoder;
        private IImageData m_ImageData;
        private int m_RectsPerRow;
        private int m_RectWidth;
        private int m_RectHeight;
        private int m_RectsRead = 0;
    }

    //
    // There should be no need to touch stuff bellow!

    interface IPixelDecoder
    {
        void Flush();
        Pixel DecodePixel();
    }

    class ChannelSettings
    {
        public static ChannelSettings Default()
        {
            return new ChannelSettings(6, 16.0f / 255.0f, 235.0f / 255.0f);
        }

        public ChannelSettings(byte bits, float lo, float hi)
        {
            if (bits < 1) throw new System.ArgumentException("bits must be at least 1.");
            else if (16 < bits) throw new System.ArgumentException("bits must be less or equal to 16.");

            m_Bits = bits;
            m_Lo = lo;
            m_Hi = hi;
        }

        public byte Bits
        {
            get
            {
                return m_Bits;
            }
        }

        public uint Decode(float value)
        {
            value -= m_Lo;
            value /= m_Hi - m_Lo;
            value *= (float)((1L << m_Bits) - 1);

            return (uint)Math.Round(value);
        }

        private byte m_Bits;
        private float m_Lo;
        private float m_Hi;
    }

    class BitsDecoder
    {
        public BitsDecoder(
            IPixelDecoder pixelDecoder,
            ChannelSettings red,
            ChannelSettings green,
            ChannelSettings blue
        )
        {
            m_PixelDecoder = pixelDecoder;
            m_Red = red;
            m_Green = green;
            m_Blue = blue;
        }

        public void Flush()
        {
            m_PixelDecoder.Flush();
            m_RedBitsLeft = 0;
            m_GreenBitsLeft = 0;
            m_BlueBitsLeft = 0;
        }

        public uint DecodeBitsToUInt32(byte bits)
        {
            uint result = 0;

            if (32 < bits) throw new System.ArgumentException("bits must be less or equal to 32.");

            while(0 < bits)
            {
                if (0 < m_BlueBitsLeft)
                {
                    byte minBits = Math.Min(bits, m_BlueBitsLeft);
                    bits -= minBits;
                    while (0 < minBits)
                    {
                        result = (result << 1) | ((m_BlueValue >> (m_Blue.Bits - 1)) & 0x1);
                        --m_BlueBitsLeft;
                        m_BlueValue = m_BlueValue << 1;
                        --minBits;
                    }
                }
                else if (0 < m_GreenBitsLeft)
                {
                    byte minBits = Math.Min(bits, m_GreenBitsLeft);
                    bits -= minBits;
                    while (0 < minBits)
                    {
                        result = (result << 1) | ((m_GreenValue >> (m_Green.Bits - 1)) & 0x1);
                        --m_GreenBitsLeft;
                        m_GreenValue = m_GreenValue << 1;
                        --minBits;
                    }
                }
                else if (0 < m_RedBitsLeft)
                {
                    byte minBits = Math.Min(bits, m_RedBitsLeft);
                    bits -= minBits;
                    while (0 < minBits)
                    {
                        result = (result << 1) | ((m_RedValue >> (m_Red.Bits - 1)) & 0x1);
                        --m_RedBitsLeft;
                        m_RedValue = m_RedValue << 1;
                        --minBits;
                    }
                }
                else
                {
                    Pixel pixel = m_PixelDecoder.DecodePixel();
                    m_RedBitsLeft = m_Red.Bits;
                    m_RedValue = m_Red.Decode(pixel.Red);
                    m_GreenBitsLeft = m_Green.Bits;
                    m_GreenValue = m_Green.Decode(pixel.Green);
                    m_BlueBitsLeft = m_Blue.Bits;
                    m_BlueValue = m_Blue.Decode(pixel.Blue);
                }
            }

            return result;
        }

        IPixelDecoder m_PixelDecoder;
        ChannelSettings m_Red;
        byte m_RedBitsLeft = 0;
        uint m_RedValue;
        ChannelSettings m_Green;
        byte m_GreenBitsLeft = 0;
        uint m_GreenValue;
        ChannelSettings m_Blue;
        byte m_BlueBitsLeft = 0;
        uint m_BlueValue;
    }

    class DataDecoder
    {
        public DataDecoder(BitsDecoder bitsDecoder)
        {
            m_BitsDecoder = bitsDecoder;
        }

        public void Flush( )
        {
            m_BitsDecoder.Flush();
        }

        public float DecodeSingle(float min, float max, byte bits)
        {
            if (bits < 2) throw new System.ArgumentException("bits must be at least 2.");
            else if(24 < bits) throw new System.ArgumentException("bits must be less or equal to 24.");

            uint data = m_BitsDecoder.DecodeBitsToUInt32(bits);

            data = (data << (23-bits)) | 0x3f000000;

            float value = BitConverter.ToSingle(BitConverter.GetBytes(data), 0);

            return value * (max - min) + min;
        }

        BitsDecoder m_BitsDecoder;
    }


    struct CameraData
    {
        public float XPosition;
        public float YPosition;
        public float ZPosition;
        public float XRotation;
        public float YRotation;
        public float ZRotation;
        public float Fov;
    }

    class CameraDataDecoder
    {
        public CameraDataDecoder(DataDecoder dataDecoder)
        {
            m_DataDecoder = dataDecoder;
        }

        public void Flush()
        {
            m_DataDecoder.Flush();
        }

        public CameraData DecodeCameraData()
        {
            CameraData result;

            result.XPosition = m_DataDecoder.DecodeSingle(-16384, 16384, 22);
            result.YPosition = m_DataDecoder.DecodeSingle(-16384, 16384, 22);
            result.ZPosition = m_DataDecoder.DecodeSingle(-16384, 16384, 22);
            result.XRotation = m_DataDecoder.DecodeSingle(-180, 180, 16);
            result.YRotation = m_DataDecoder.DecodeSingle(-180, 180, 16);
            result.ZRotation = m_DataDecoder.DecodeSingle(-180, 180, 16);
            result.Fov = m_DataDecoder.DecodeSingle(0, 180, 14);

            return result;
        }

        private DataDecoder m_DataDecoder;
    }
}
