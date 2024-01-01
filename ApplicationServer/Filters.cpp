#include "Filters.h"

cv::Mat Rotate::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    cv::Mat newImg;
    if (params[0] == "90")
    {
        cv::rotate(img, newImg, cv::ROTATE_90_CLOCKWISE);
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
    auto startSingle = std::chrono::high_resolution_clock::now();
    GrayscaleSingleThreaded(img);
    auto stopSingle = std::chrono::high_resolution_clock::now();
    auto singleDuration = std::chrono::duration_cast<std::chrono::microseconds>(stopSingle - startSingle);
    auto startMulti = std::chrono::high_resolution_clock::now();
    cv::Mat grayscaleImg = GrayscaleMultiThreaded(img);
    auto stopMulti = std::chrono::high_resolution_clock::now();
    auto multiDuration = std::chrono::duration_cast<std::chrono::microseconds>(stopMulti - startMulti);
    std::cout << singleDuration.count() << " - duration of single threaded in microseconds" << std::endl;
    std::cout << multiDuration.count() << " - duration of multi threaded in microseconds" << std::endl;
    return grayscaleImg;
}

cv::Mat Greyscale::GrayscaleSingleThreaded(cv::Mat& img)
{
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
    //https://support.ptc.com/help/mathcad/r9.0/en/index.html#page/PTC_Mathcad_Help/example_grayscale_and_color_in_images.html
    return newImg;
}

cv::Mat Greyscale::GrayscaleMultiThreaded(cv::Mat& img)
{
    cv::Mat newImg = img;
    int size = std::round((double)img.rows / NUM_THREADS);
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        std::thread t1(&Greyscale::GrayscaleThread, this, std::ref(newImg), newImg, i * size, size);
        threads.push_back(std::move(t1));
    }

    for (int i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }
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
