/* Reorganize (c) 2006 Bjørn Lindeijer
 * License: GPL, v2 or later
 */

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.util.Vector;
import java.io.IOException;
import java.io.File;
import javax.imageio.ImageIO;

/**
 * Tool to reorganize the hair spritesets.
 */
public class Reorganize
{
    private static final int SPRITE_WIDTH = 60;
    private static final int SPRITE_HEIGHT = 60;
    private static final int FRAMES = 10;
    private static final int DIRECTIONS = 4;

    private static final int HAIR_COLORS = 10;
    private static final int HAIR_FRAMES = 9;
    private static final int HAIR_SPRITE_WIDTH = 40;
    private static final int HAIR_SPRITE_HEIGHT = 40;

    private static final int TRANSPARENT = new Color(255, 0, 255).getRGB();

    public static void main(String[] arg)
    {
        if (arg.length != 2) {
            System.out.println("Usage:\n   java Reorganize [source] [target]");
            return;
        }

        BufferedImage source = null;
        try {
            source = ImageIO.read(new File(arg[0]));
        } catch (IOException e) {
            System.out.println("Error while trying to read " + arg[0] + ".");
            e.printStackTrace();
            System.exit(1);
        }

        // Read the existing frames into a vector
        Vector<BufferedImage> spriteSet = gridCut(source,
                HAIR_SPRITE_WIDTH, HAIR_SPRITE_HEIGHT,
                HAIR_FRAMES, HAIR_COLORS);

        // Determine minimal rectangle that can still contain the contents of
        // any frame
        /*
        Rectangle cropRect = minimumCropRect(spriteSet);

        if (cropRect == null) {
            System.out.println(
                    "Error: no optimal crop rect could be determined.");
            System.exit(1);
        }

        // Make crop rect one pixel larger (since we want an inclusive rect)
        cropRect.add(
                cropRect.x + cropRect.width + 1,
                cropRect.y + cropRect.height + 1);

        System.out.println(arg[0] + ": width=\"" +
                cropRect.width + "\" height=\"" + cropRect.height + "\"");
        */

        filterHairstyle(spriteSet);

        BufferedImage target = gridDraw(
                spriteSet,
                new Rectangle(0, 0, HAIR_SPRITE_WIDTH, HAIR_SPRITE_HEIGHT),
                HAIR_FRAMES - 4, HAIR_COLORS);

        // Save the target image
        try {
            ImageIO.write(target, "png", new File(arg[1]));
        } catch (IOException e) {
            System.out.println("Error while trying to write " + arg[1] + ".");
            e.printStackTrace();
            System.exit(1);
        }
    }

    private static Vector<BufferedImage> gridCut(
            BufferedImage source,
            int width, int height, int xFrames, int yFrames)
    {
        Vector<BufferedImage> spriteSet = new Vector<BufferedImage>();

        for (int y = 0; y < yFrames; y++) {
            for (int x = 0; x < xFrames; x++) {
                BufferedImage sprite = source.getSubimage(
                        x * width,
                        y * height,
                        width,
                        height);

                spriteSet.add(sprite);
            }
        }

        return spriteSet;
    }

    private static BufferedImage gridDraw(Vector<BufferedImage> spriteSet,
            Rectangle cropRect, int xFrames, int yFrames)
    {
        // Create a new image
        BufferedImage target = new BufferedImage(
                xFrames * cropRect.width,
                yFrames * cropRect.height,
                BufferedImage.TYPE_INT_ARGB);

        // Draw the frames onto the target image
        Graphics g = target.getGraphics();
        for (int y = 0; y < yFrames; y++) {
            for (int x = 0; x < xFrames; x++) {
                g.drawImage(
                        spriteSet.get(x + xFrames * y).getSubimage(
                            cropRect.x,
                            cropRect.y,
                            cropRect.width,
                            cropRect.height),
                        x * cropRect.width,
                        y * cropRect.height,
                        null);
            }
        }

        return target;
    }

    private static Rectangle minimumCropRect(Vector<BufferedImage> spriteSet)
    {
        Rectangle cropRect = null;

        for (BufferedImage sprite : spriteSet) {
            Rectangle frameCropRect = determineCropRect(sprite);

            if (cropRect == null) {
                cropRect = frameCropRect;
            } else {
                cropRect.add(frameCropRect);
            }
        }

        return cropRect;
    }

    private static Rectangle determineCropRect(BufferedImage image)
    {
        // Loop through all the pixels, ignoring transparent ones.
        Rectangle rect = null;

        for (int y = 0; y < image.getHeight(); y++) {
            for (int x = 0; x < image.getWidth(); x++) {
                int color = image.getRGB(x, y);

                if (color != TRANSPARENT && (color & 0xFF000000) != 0) {
                    if (rect == null) {
                        rect = new Rectangle(x, y, 0, 0);
                    } else {
                        rect.add(x, y);
                    }
                }
            }
        }

        return rect;
    }

    private static void filterHairstyle(Vector<BufferedImage> spriteSet)
    {
        // Remove frame 1, 2, 6 and 7 from each color
        for (int i = HAIR_COLORS - 1; i >= 0; i--) {
            spriteSet.remove(i * HAIR_FRAMES + 7);
            spriteSet.remove(i * HAIR_FRAMES + 6);
            spriteSet.remove(i * HAIR_FRAMES + 2);
            spriteSet.remove(i * HAIR_FRAMES + 1);
        }
    }
}
