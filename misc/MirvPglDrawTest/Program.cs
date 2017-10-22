using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using advancedfx.PGL;

namespace MirvPglDrawTest
{
    class ImageData : IImageData
    {
        public ImageData(string fileName, int left, int top)
        {
            m_Bitmap = new Bitmap(fileName);

            m_Left = left;
            m_Top = top;
        }

        int m_Left;
        int m_Top;
        Bitmap m_Bitmap;

        Pixel IImageData.GetPixel(int x, int y)
        {
            if(x < 0 || y < 0 || m_Left + x >= m_Bitmap.Width || m_Top +y >= m_Bitmap.Height)
            {
                throw new ApplicationException("x,y out of allowed bounds.");
            }

            Color color = m_Bitmap.GetPixel(m_Left + x, m_Top + y);

            Pixel result;

            result.Red = color.R / 255.0f;
            result.Green = color.G / 255.0f;
            result.Blue = color.B / 255.0f;

            return result;
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            EasyDecoder decoder = EasyDecoder.Default();
            EasyData data = null;

            try
            {
                data = decoder.Decode(new ImageData(args[0], 10, 75));
            }
            catch(System.Exception e)
            {
                Console.Out.WriteLine("Error " + e.ToString());
                data = null;
            }

            if(null != data)
            {
                CameraData cameraData = data.CameraData;

                Console.Out.WriteLine("X={0}, Y={1}, Z={2}, XR={3}, YR={4}, ZR={5}, FOV={6}", cameraData.XPosition, cameraData.YPosition, cameraData.ZPosition, cameraData.XRotation, cameraData.YRotation, cameraData.ZRotation, cameraData.Fov);
            }

            System.Console.ReadLine();
        }
    }
}
