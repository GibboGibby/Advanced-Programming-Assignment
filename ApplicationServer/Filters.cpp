#include "Filters.h"
/// <summary>
/// Rotate Filter
/// </summary>
/// <param name="img">Image</param>
/// <param name="params">Parameters</param>
/// <returns>Rotated Image</returns>
cv::Mat Rotate::RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*)
{
    cv::Mat newImg;
    // Uses OpenCV rotate function based on the parameters supplied
    if (params[0] == "90" || params[0] == "-270")
    {
        cv::rotate(img, newImg, cv::ROTATE_90_CLOCKWISE);
    }
    else if (params[0] == "180" || params[0] == "-180")
    {
        cv::rotate(img, newImg, cv::ROTATE_180);
    }
    else if (params[0] == "-90" || params[0] == "270")
    {
        cv::rotate(img, newImg, cv::ROTATE_90_COUNTERCLOCKWISE);
    }
    else 
        throw(new InvalidArgumentException(params[0].c_str()));
    return newImg;
}



/// <summary>
/// Grayscale Filter
/// </summary>
/// <param name="img">Image</param>
/// <param name="params">Parameters</param>
/// <returns>GrayscaleImage</returns>
cv::Mat Greyscale::RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*)
{
    // Perform Grayscale filter single or multi-threaded
    if (params.size() == 0)
    {
        return GrayscaleMultiThreaded(img);
    }
    if (params[0] == "single") return GrayscaleSingleThreaded(img);
    else return GrayscaleMultiThreaded(img);

}

cv::Mat Greyscale::GrayscaleSingleThreaded(cv::Mat& img)
{
    auto startSingle = std::chrono::high_resolution_clock::now();
    cv::Mat newImg = img.clone();
    for (int i = 0; i < newImg.rows; i++)
    {
        for (int j = 0; j < newImg.cols; j++)
        {
            cv::Vec3b pixel = newImg.at<cv::Vec3b>(i, j);

            //BGR
            //0.299 * red | 0.587 * green | 0.114 * blue
            // Add values into the pixel val and then update the image
            int pixelVal = 0;
            pixelVal += (int)pixel[0] * 0.114;
            pixelVal += (int)pixel[1] * 0.587;
            pixelVal += (int)pixel[2] * 0.299;
            pixel[0] = (uchar)pixelVal;
            pixel[1] = (uchar)pixelVal;
            pixel[2] = (uchar)pixelVal;

            newImg.at<cv::Vec3b>(i, j) = pixel;
        }
    }

    auto stopSingle = std::chrono::high_resolution_clock::now();
    auto singleDuration = std::chrono::duration_cast<std::chrono::microseconds>(stopSingle - startSingle);
    std::cout << singleDuration.count() << " - duration of single threaded in microseconds" << std::endl;
    //https://support.ptc.com/help/mathcad/r9.0/en/index.html#page/PTC_Mathcad_Help/example_grayscale_and_color_in_images.html
    return newImg;
}

cv::Mat Greyscale::GrayscaleMultiThreaded(cv::Mat& img)
{
    auto startMulti = std::chrono::high_resolution_clock::now();
    // Use multi threaded processing function
    // Passing in a grayscale function on each pixel
    cv::Mat newImg = GibCore::MultithreadedImageProcessing(_mutex, img, NUM_THREADS, [](cv::Mat& img, int x, int y) {
            cv::Vec3b pixel = img.at<cv::Vec3b>(x, y);
            int pixelVal = 0;
            pixelVal += (int)pixel[0] * 0.114;
            pixelVal += (int)pixel[1] * 0.587;
            pixelVal += (int)pixel[2] * 0.299;
            pixel[0] = (uchar)pixelVal;
            pixel[1] = (uchar)pixelVal;
            pixel[2] = (uchar)pixelVal;
            img.at<cv::Vec3b>(x, y) = pixel;
        });

    auto stopMulti = std::chrono::high_resolution_clock::now();
    auto multiDuration = std::chrono::duration_cast<std::chrono::microseconds>(stopMulti - startMulti);
    std::cout << multiDuration.count() << " - duration of multi threaded in microseconds" << std::endl;

    return newImg;
}


/// <summary>
/// Flips an image
/// </summary>
/// <param name="img">image</param>
/// <param name="params">parameters</param>
/// <returns>Flipped Image</returns>
cv::Mat Flip::RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*)
{
    // Gets appropriate flip code from the params
    int flipCode = 0;
    if (params[0] == "horizontal") flipCode = 0;
    else if (params[0] == "vertical") flipCode = 1;
    else if (params[0] == "both") flipCode = -1;
    else throw(new InvalidArgumentException(params[0].c_str()));
    cv::Mat flippedImg;
    // Flips images based on flipped code
    cv::flip(img, flippedImg, flipCode);
    return flippedImg;
}

/// <summary>
/// Resizes image
/// </summary>
/// <param name="img">Image</param>
/// <param name="params">Parameters</param>
/// <returns>Resized Image</returns>
cv::Mat Resize::RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*)
{
    // Gets new image size from params
    cv::Mat resizedImg;
    int x, y;
    try {
    x = stoi(params[0]);
    y = stoi(params[1]);
    }
    catch (std::exception e)
    {
        throw (new InvalidConvertException("Size", "integer"));
    }
    cv::Size size(x,y);
    // Resizes image using OpenCV functions
    cv::resize(img, resizedImg, size);
    return resizedImg;
}

/// <summary>
/// Crops image
/// </summary>
/// <param name="img">Image</param>
/// <param name="params">Parameters</param>
/// <returns>Cropped image</returns>
cv::Mat Crop::RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*)
{
    cv::Mat croppedImg;
    int startX;
    int startY;
    int endX;
    int endY;
    // Uses number of params to decide where the image is cropped and if it is cropped from 0,0
    try {

        if (params.size() < 4)
        {
            startX = 0;
            startY = 0;
            endX = stoi(params[0]);
            endY = stoi(params[1]);
        }
        else
        {
            startX = stoi(params[0]);
            startY = stoi(params[1]);
            endX = stoi(params[2]);
            endY = stoi(params[3]);
        }
    }
    catch (std::exception e)
    {
        throw (new InvalidConvertException("Size", "integer"));
    }

    // Uses rect to crop image
    cv::Rect rect(startX, startY, endX, endY);
    croppedImg = img(rect);
    return croppedImg;
}
/// <summary>
/// Box Blurs an Image
/// </summary>
/// <param name="img">Image</param>
/// <param name="params">Parameters</param>
/// <returns>Blurred Image</returns>
cv::Mat BoxBlur::RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*)
{
    // If no parameters are defined use set size
    int sizeX;
    int sizeY;
    if (params.size() == 0)
    {
        sizeX = 3;
        sizeY = 3;
    }
    else
    {
        try {
            sizeX = stoi(params[0]);
            sizeY = stoi(params[1]);
        }
        catch(std::exception e) {
            throw (new InvalidConvertException("Box blur size", "integer"));
        }
    }
    // If parallel is set, use multithreaded option
    if (params.size() == 3)
    {
        if (params[2] == "parallel")
        {
            return BoxBlurMutliThreaded(img, sizeX, sizeY);
        }
    }
    return BoxBlurSingleThreaded(img, sizeX, sizeY);
}

cv::Mat BoxBlur::BoxBlurSingleThreaded(cv::Mat& img, int sizeX, int sizeY)
{
    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat boxBlurImg = img;

    for (int i = 0; i < boxBlurImg.rows; i++)
    {

        for (int j = 0; j < boxBlurImg.cols; j++)
        {
            int red = 0;
            int green = 0;
            int blue = 0;
            int count = 0;
            for (int x = -((sizeX - 1) / 2); x < (((sizeX - 1) / 2) + 1); x++)
            {
                for (int y = -((sizeX - 1) / 2); y < (((sizeX - 1) / 2) + 1); y++)
                {
                    // Loop through all of the surrounding pixels of an image (based on the size specific in the size)
                    int tempX = i + x;
                    int tempY = j + y;
                    if (tempX < 0 || tempX >= boxBlurImg.rows || tempY < 0 || tempY >= boxBlurImg.cols) continue;
                    cv::Vec3b temp = boxBlurImg.at<cv::Vec3b>(tempX, tempY);
                    //std::cout << "Current x val - " << i + x << std::endl;
                    //std::cout << "Current y val - " << j + y << std::endl;
                    blue += (int)temp[0];
                    green += (int)temp[1];
                    red += (int)temp[2];
                    count++;
                }
            }
            //sum / count;
            // Calculates the average count of the image values based on the number of pixels looped through and updates image
            cv::Vec3b temp;
            red = red / count;
            green = green / count;
            blue = blue / count;
            temp[0] = (uchar)blue;
            temp[1] = (uchar)green;
            temp[2] = (uchar)red;
            boxBlurImg.at<cv::Vec3b>(i, j) = temp;
            
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "duration of box blur - " << duration.count();
    //cv::blur(img, boxBlurImg, cv::Size(3, 3));
    //https://en.wikipedia.org/wiki/Box_blur
    return boxBlurImg;
}

cv::Mat BoxBlur::BoxBlurMutliThreaded(cv::Mat& img, int sizeX, int sizeY)
{
    cv::Mat newImg = img.clone();
    int size = std::round((double)img.rows / NUM_THREADS);
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        std::thread t1(&BoxBlur::BoxBlurThread, this, std::ref(newImg), newImg, i * size, size, sizeX, sizeY);
        threads.push_back(std::move(t1));
    }

    for (int i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "duration of box blur - " << duration.count();
    return newImg;
}

void BoxBlur::BoxBlurThread(cv::Mat& origImg, cv::Mat tempImg, int startPos, int size, int sizeX, int sizeY)
{
    cv::Mat boxBlurImg = origImg;
    for (int i = startPos; i < startPos + size; i++)
    {
        if (i > tempImg.cols) continue;
        for (int j = 0; j < tempImg.cols; j++)
        {
            int red = 0;
            int green = 0;
            int blue = 0;
            int count = 0;
            for (int x = -((sizeX - 1) / 2); x < (((sizeX - 1) / 2) + 1); x++)
            {
                for (int y = -((sizeX - 1) / 2); y < (((sizeX - 1) / 2) + 1); y++)
                {
                    int tempX = i + x;
                    int tempY = j + y;
                    if (tempX < 0 || tempX >= boxBlurImg.rows || tempY < 0 || tempY >= boxBlurImg.cols) continue;
                    cv::Vec3b temp = boxBlurImg.at<cv::Vec3b>(tempX, tempY);
                    //std::cout << "Current x val - " << i + x << std::endl;
                    //std::cout << "Current y val - " << j + y << std::endl;
                    blue += (int)temp[0];
                    green += (int)temp[1];
                    red += (int)temp[2];
                    count++;
                }
            }
            //sum / count;
            cv::Vec3b temp;
            red = red / count;
            green = green / count;
            blue = blue / count;
            temp[0] = (uchar)blue;
            temp[1] = (uchar)green;
            temp[2] = (uchar)red;
            boxBlurImg.at<cv::Vec3b>(i, j) = temp;
        }
    }
}

/// <summary>
/// Sharpens an image
/// </summary>
/// <param name="img">Image</param>
/// <param name="params">Parameters</param>
/// <returns>Sharpened Image</returns>
cv::Mat Sharpening::RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*)
{
    // Defines a sharpening kernel and runs filter2D using sharpened kernel
    cv::Mat sharpenedImg;
    //cv::InputArray kernel([[-1, -1, -1], [-1, 9, -1], [-1, -1, -1]] );
    cv::Mat sharpeningKernel = (cv::Mat_<double>(3, 3) << -1, -1, -1,
        -1, 9, -1,
        -1, -1, -1);
    cv::filter2D(img, sharpenedImg, -1, sharpeningKernel);
    //https://answers.opencv.org/question/216383/how-could-do-sharpness-images/
    return sharpenedImg;
}

/// <summary>
/// Adjusts the image brightness
/// </summary>
/// <param name="img">Image</param>
/// <param name="params">Parameters</param>
/// <returns>Adjusted image</returns>
cv::Mat BrightnessAdjust::RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*)
{
    int brightness;
    try {
        brightness = stoi(params[0]);
        if (brightness < -255 || brightness > 255) {
            std::stringstream ss;
            ss << brightness;
            throw(new InvalidRangeException(ss.str().c_str(), "-255", "255"));
        }
    }
    catch (std::exception& err)
    {
        throw(new InvalidConvertException("Brightness", "integer"));
    }
    return BrightnessAdjustMultithreaded(img, brightness);
}

cv::Mat BrightnessAdjust::BrightnessAdjustMultithreaded(cv::Mat& img, int brightness) 
{
    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat newImg = GibCore::MultithreadedImageProcessing(_mutex, img, NUM_THREADS, [brightness](cv::Mat& img, int x, int y) {
            // Loops through the image adding the brightness values to each channel
            cv::Vec3b pixel = img.at<cv::Vec3b>(x, y);
            int blue = pixel[0] + brightness;
            int green = pixel[1] + brightness;
            int red = pixel[2] + brightness;
            // Clamps the values between 0 and 255
            pixel[0] = GibCore::Clamp2(blue, 0, 255);
            pixel[1] = GibCore::Clamp2(green, 0, 255);
            pixel[2] = GibCore::Clamp2(red, 0, 255);
            // Updates image
            img.at<cv::Vec3b>(x, y) = pixel;
        });

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "duration of brightness adjust - " << duration.count();

    return newImg;
}

cv::Mat GammaCorrection::RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*)
{
    double gamma;
    try {
        gamma = std::stod(params[0]);
    }
    catch (std::exception& err)
    {
        throw(new InvalidConvertException("Gamma", "double"));
    }
    std::cout << "This is the gamme value - " << gamma << std::endl;
    cv::Mat newImg = GibCore::MultithreadedImageProcessing(_mutex, img, NUM_THREADS, [gamma](cv::Mat& img, int x, int y) {
        double gammaCorrection = 1 / gamma;
        cv::Vec3b pixel = img.at<cv::Vec3b>(x, y);
        // Gets value of pixel and runs gamma correction formula
        double blue = (double)pixel[0] / 255;
        double green = (double)pixel[1] / 255;
        double red = (double)pixel[2] / 255;
        pixel[0] = (uchar)(255 * std::pow(blue, gammaCorrection));
        pixel[1] = (uchar)(255 * std::pow(green, gammaCorrection));;
        pixel[2] = (uchar)(255 * std::pow(red, gammaCorrection));;
        img.at<cv::Vec3b>(x, y) = pixel;
        });
    //https://www.dfstudios.co.uk/articles/programming/image-programming-algorithms/image-processing-algorithms-part-6-gamma-correction/
    return newImg;
}

cv::Mat ContrastAdjust::RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*)
{
    try 
    {
        int contrast = stoi(params[0]);
        if (contrast < -255 || contrast > 255)
        {
            std::stringstream ss;
            ss << contrast;
            throw(new InvalidRangeException(ss.str().c_str(), "-255", "255"));
        }
    }
    catch (std::exception& e)
    {
        throw(new InvalidConvertException("contrast", "integer"));
    }
    int factor = (259 * (contrast + 255)) / (255 * (259 - contrast));
    cv::Mat newImg = GibCore::MultithreadedImageProcessing(_mutex, img, NUM_THREADS, [factor](cv::Mat& img, int x, int y) 
        {
            // Gets value of pixel and runs contrast adjusting formula
            cv::Vec3b pixel = img.at<cv::Vec3b>(x, y);
            int blue = pixel[0];
            int green = pixel[1];
            int red = pixel[2];
            blue = factor * (blue - 128) + 128;
            green = factor * (green - 128) + 128;
            red = factor * (red - 128) + 128;
            // Clamps image
            pixel[0] = GibCore::Clamp2(blue, 0, 255);
            pixel[1] = GibCore::Clamp2(green, 0, 255);
            pixel[2] = GibCore::Clamp2(red, 0, 255);
            img.at<cv::Vec3b>(x, y) = pixel;
        });
    //https://www.dfstudios.co.uk/articles/programming/image-programming-algorithms/image-processing-algorithms-part-5-contrast-adjustment/
    return newImg;  
}
