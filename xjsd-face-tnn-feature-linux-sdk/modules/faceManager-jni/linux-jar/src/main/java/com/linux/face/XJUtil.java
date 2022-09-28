package com.linux.face;


import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.*;


/**
 * Author:deepin
 * Date:2022/9/20 下午7:54
 */
public class XJUtil {

    public static int BindBytesToJNI(byte[] bytes) {
        return 0;
    }

    public static int SetBytesToJNI(byte[] bytes) {
        return 0;
    }

    /**
     * 读取一张图片的RGB值
     */
    public static XJImage2D getImage2D(String imagePath) {
        // TODO: 2022/9/20
        File file = new File(imagePath);
        BufferedImage bi = null;
        try {
            bi = ImageIO.read(file);
        } catch (IOException e) {
            e.printStackTrace();
        }
        assert bi != null;
        int width = bi.getWidth();
        int height = bi.getHeight();
        byte[] data = new byte[width * height * 3];
        int minX = bi.getMinX();
        int minY = bi.getMinY();
        int index = 0;
        for (int y = minY; y < height; y++) {
            for (int x = minX; x < width; x++) {
                //获取包含这个像素的颜色信息的值, int型
                int pixel = bi.getRGB(x, y);
                //从pixel中获取rgb的值
                data[index++] = (byte) (pixel & 0xff);
                data[index++] = (byte) ((pixel & 0xff00) >> 8);
                data[index++] = (byte) ((pixel & 0xff0000) >> 16);
            }
        }

        return new XJImage2D(data, width, height, 0, XJImage2D.IMAGE_FORMAT_TYPE_BGR);
    }

    // TODO: 2022/9/20
    public static byte[] ReadAssetsFilesToBytes(String assetsFileName) {
        InputStream in = null;
        ByteArrayOutputStream bos = null;
        try {
//get input stream of assets file
            in = new FileInputStream(assetsFileName);
//new a output stream
            bos = new ByteArrayOutputStream();
//new a buffer array
            int len = -1;
            byte buf[] = new byte[in.available()];
//read file stream form input stream to output stream
            while ((len = in.read(buf)) != -1) {
                bos.write(buf, 0, len);
            }
            bos.close();
            bos = null;
            in.close();
            in = null;
            return buf;
        } catch (IOException e) {
            e.printStackTrace();
            if (null == in) {
                return null;
            }
        }
        return null;
    }

    public static String SaveToFile(byte[] data, String ParentFile, String fileName, String suffix) {
        File mFile = new File(ParentFile, fileName + "." + suffix);
        FileOutputStream output = null;
        try {
            output = new FileOutputStream(mFile);
            output.write(data);
        } catch (IOException e) {
            e.printStackTrace();
            return "!~Failed~!";
        } finally {
            if (null != output) {
                try {
                    output.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return mFile.getPath();
    }

}
