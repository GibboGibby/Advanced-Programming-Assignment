#include "Filters.h"

cv::Mat Rotate::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    cv::Mat newImg;
    if (params[0] == "90")
    {
        cv::rotate(img, newImg, cv::ROTATE_90_CLOCKWISE);
    }
    else if (params[0] == "180")
    {
        cv::rotate(img, newImg, cv::ROTATE_180);
    }
    else if (params[0] == "-90" || params[0] == "270")
    {
        cv::rotate(img, newImg, cv::ROTATE_90_COUNTERCLOCKWISE);
    }
    else
        cv::rotate(img, newImg, cv::ROTATE_90_COUNTERCLOCKWISE);
    return newImg;
}
Rotate::Rotate()
{

}

Greyscale::Greyscale()
{

}

cv::Mat Greyscale::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    //return GrayscaleSingleThreaded(img);
    if (params.size() == 0)
    {
        return GrayscaleMultiThreaded(img);
    }
    if (params[0] == "parallel") return GrayscaleMultiThreaded(img);
    else if (params[0] == "single") return GrayscaleSingleThreaded(img);
    else if (params[0] == "testing")
    {
        return GibCore::FilterLambda(img, [](cv::Mat& img, int x, int y) {
            cv::Vec3b pixel = img.at<cv::Vec3b>(x, y);
            int pixelVal = 0;
            pixelVal += (int)pixel[0] * 0.114;
            pixelVal += (int)pixel[1] * 0.587;
            pixelVal += (int)pixel[2] * 0.299;
            pixelVal /= 3;
            pixel[0] = (uchar)pixelVal;
            pixel[1] = (uchar)pixelVal;
            pixel[2] = (uchar)pixelVal;
            img.at<cv::Vec3b>(x, y) = pixel;
            });
    }
    
    
    

    
    
}

cv::Mat Greyscale::GrayscaleSingleThreaded(cv::Mat& img)
{
    auto startSingle = std::chrono::high_resolution_clock::now();
    cv::Mat newImg = img;
    for (int i = 0; i < newImg.rows; i++)
    {
        for (int j = 0; j < newImg.cols; j++)
        {
            cv::Vec3b pixel = newImg.at<cv::Vec3b>(i, j);

            //BGR
            //0.299 * red | 0.587 * green | 0.114 * blue
            int pixelVal = 0;
            pixelVal += (int)pixel[0] * 0.114;
            pixelVal += (int)pixel[1] * 0.587;
            pixelVal += (int)pixel[2] * 0.299;
            pixelVal /= 3;
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
    cv::Mat newImg = GibCore::MultithreadedImageProcessing(mutex, img, NUM_THREADS, [](cv::Mat& img, int x, int y) {
            cv::Vec3b pixel = img.at<cv::Vec3b>(x, y);
            int pixelVal = 0;
            pixelVal += (int)pixel[0] * 0.114;
            pixelVal += (int)pixel[1] * 0.587;
            pixelVal += (int)pixel[2] * 0.299;
            pixelVal /= 3;
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

void Greyscale::GrayscaleThread(cv::Mat& origImg, cv::Mat tempImg, int startPos, int size)
{
    for (int i = startPos; i < startPos + size; i++)
    {
        if (i > tempImg.cols) continue;
        for (int j = 0; j < tempImg.cols; j++)
        {
            cv::Vec3b pixel = tempImg.at<cv::Vec3b>(i, j);
            int pixelVal = 0;
            pixelVal += (int)pixel[0] * 0.114;
            pixelVal += (int)pixel[1] * 0.587;
            pixelVal += (int)pixel[2] * 0.299;
            pixelVal /= 3;
            pixel[0] = (uchar)pixelVal;
            pixel[1] = (uchar)pixelVal;
            pixel[2] = (uchar)pixelVal;
            tempImg.at<cv::Vec3b>(i, j) = pixel;
        }
    }

    
    mutex.lock();
    for (int i = startPos; i < startPos + size; i++)
    {
        for (int j = 0; j < tempImg.cols; j++)
        {
            origImg.at<cv::Vec3b>(i, j) = tempImg.at<cv::Vec3b>(i, j);
        }
    }
    mutex.unlock();
}

Flip::Flip()
{
}

cv::Mat Flip::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    int flipCode = 0;
    if (params[0] == "horizontal") flipCode = 0;
    else if (params[0] == "vertical") flipCode = 1;
    else if (params[0] == "both") flipCode = -1;
    cv::Mat flippedImg;
    cv::flip(img, flippedImg, flipCode);
    return flippedImg;
}

cv::Mat Resize::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    cv::Mat resizedImg;
    int x = stoi(params[0]);
    int y = stoi(params[1]);
    cv::Size size(x,y);
    cv::resize(img, resizedImg, size);
    return resizedImg;
}

cv::Mat Crop::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    cv::Mat croppedImg;
    int startX;
    int startY;
    int endX;
    int endY;
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

    cv::Rect rect(startX, startY, endX, endY);
    croppedImg = img(rect);
    return croppedImg;
}

cv::Mat BoxBlur::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    int sizeX;
    int sizeY;
    if (params.size() == 0)
    {
        sizeX = 3;
        sizeY = 3;
    }
    else
    {
        sizeX = stoi(params[0]);
        sizeY = stoi(params[1]);
    }
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
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "duration of box blur - " << duration.count();
    //cv::blur(img, boxBlurImg, cv::Size(3, 3));
    //https://en.wikipedia.org/wiki/Box_blur
    return boxBlurImg;
}

cv::Mat BoxBlur::BoxBlurMutliThreaded(cv::Mat& img, int sizeX, int sizeY)
{
    cv::Mat newImg = img;
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
    cv::Mat boxBlurImg = tempImg;
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
    mutex.lock();
    for (int i = startPos; i < startPos + size; i++)
    {
        for (int j = 0; j < tempImg.cols; j++)
        {
            origImg.at<cv::Vec3b>(i, j) = tempImg.at<cv::Vec3b>(i, j);
        }
    }
    mutex.unlock();
}

cv::Mat Sharpening::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    cv::Mat sharpenedImg;
    //cv::InputArray kernel([[-1, -1, -1], [-1, 9, -1], [-1, -1, -1]] );
    cv::Mat sharpeningKernel = (cv::Mat_<double>(3, 3) << -1, -1, -1,
        -1, 9, -1,
        -1, -1, -1);
    cv::filter2D(img, sharpenedImg, -1, sharpeningKernel);
    //https://answers.opencv.org/question/216383/how-could-do-sharpness-images/
    return sharpenedImg;
}

cv::Mat BrightnessAdjust::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    int brightness = stoi(params[0]);
    return BrightnessAdjustMultithreaded(img, brightness);
}

cv::Mat BrightnessAdjust::BrightnessAdjustMultithreaded(cv::Mat& img, int brightness)
{
    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat newImg = GibCore::MultithreadedImageProcessing(mutex, img, NUM_THREADS, [brightness](cv::Mat& img, int x, int y) {
            cv::Vec3b pixel = img.at<cv::Vec3b>(x, y);
            int blue = pixel[0] + brightness;
            int green = pixel[1] + brightness;
            int red = pixel[2] + brightness;
            pixel[0] = GibCore::Clamp2(blue, 0, 255);
            pixel[1] = GibCore::Clamp2(green, 0, 255);
            pixel[2] = GibCore::Clamp2(red, 0, 255);
            img.at<cv::Vec3b>(x, y) = pixel;
        });

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "duration of brightness adjust - " << duration.count();

    return newImg;
}

cv::Mat GammaCorrection::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    double gamma = std::stod(params[0]);
    std::cout << "This is the gamme value - " << gamma << std::endl;
    cv::Mat newImg = GibCore::MultithreadedImageProcessing(mutex, img, NUM_THREADS, [gamma](cv::Mat& img, int x, int y) {
        double gammaCorrection = 1 / gamma;
        cv::Vec3b pixel = img.at<cv::Vec3b>(x, y);
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

cv::Mat ContrastAdjust::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    int contrast = stoi(params[0]);
    int factor = (259 * (contrast + 255)) / (255 * (259 - contrast));
    cv::Mat newImg = GibCore::MultithreadedImageProcessing(mutex, img, NUM_THREADS, [factor](cv::Mat& img, int x, int y) 
        {
            cv::Vec3b pixel = img.at<cv::Vec3b>(x, y);
            int blue = pixel[0];
            int green = pixel[1];
            int red = pixel[2];
            blue = factor * (blue - 128) + 128;
            green = factor * (green - 128) + 128;
            red = factor * (red - 128) + 128;
            pixel[0] = GibCore::Clamp2(blue, 0, 255);
            pixel[1] = GibCore::Clamp2(green, 0, 255);
            pixel[2] = GibCore::Clamp2(red, 0, 255);
            img.at<cv::Vec3b>(x, y) = pixel;
        });
    //https://www.dfstudios.co.uk/articles/programming/image-programming-algorithms/image-processing-algorithms-part-5-contrast-adjustment/
    return newImg;  
}
