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
 * Very simple tool to reorganize the monster spritesets.
 */
public class Reorganize
{
    private static final int SPRITE_WIDTH = 60;
    private static final int SPRITE_HEIGHT = 60;
    private static final int FRAMES = 10;
    private static final int DIRECTIONS = 4;
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

        Rectangle cropRect = null;

        // Read the existing frames into a vector and determine minimal
        // rectangle that still can contain the contents of any frame.
        Vector<BufferedImage> spriteSet = new Vector<BufferedImage>();
        for (int x = 0; x < DIRECTIONS; x++) {
            for (int y = 0; y < FRAMES; y++) {
                BufferedImage sprite = source.getSubimage(
                        x * SPRITE_WIDTH,
                        y * SPRITE_HEIGHT,
                        SPRITE_WIDTH,
                        SPRITE_HEIGHT);

                spriteSet.add(sprite);

                Rectangle frameCropRect = determineCropRect(sprite);

                if (cropRect == null) {
                    cropRect = frameCropRect;
                } else {
                    cropRect.add(frameCropRect);
                }
            }
        }

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

        // Create a new image (with frame direction flipped)
        BufferedImage target = new BufferedImage(
                FRAMES * cropRect.width,
                DIRECTIONS * cropRect.height,
                BufferedImage.TYPE_INT_ARGB);

        // Draw the frames onto the target image
        Graphics g = target.getGraphics();
        for (int y = 0; y < DIRECTIONS; y++) {
            for (int x = 0; x < FRAMES; x++) {
                g.drawImage(
                        spriteSet.get(x + FRAMES * y).getSubimage(
                            cropRect.x,
                            cropRect.y,
                            cropRect.width,
                            cropRect.height),
                        x * cropRect.width,
                        y * cropRect.height,
                        null);
            }
        }

        // Save the target image
        try {
            ImageIO.write(target, "png", new File(arg[1]));
        } catch (IOException e) {
            System.out.println("Error while trying to write " + arg[1] + ".");
            e.printStackTrace();
            System.exit(1);
        }
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
}
