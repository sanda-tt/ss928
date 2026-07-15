This sample is used to evaluate the correlation between sensor pixels.

Caution:
1. Please use colorchecker, select the third gray block as the statistical area. Please check the size of the ROI area does not exceed 80 x 80. Otherwise, the size of the ROI area may exceed the memory limit when there are many frames.
2. This sample collects pixel values in the time domain. The number of frames must be at least 5000.
3. Makesure that the light and the scene does not change during the statistics.
4. The illumination will change periodically during measurement. It is recommended that the frame interval be set to 15. The larger the interval, the longer the measurement takes.
5. The larger average correlation coefficient, the stronger correlation between pixels. If the statistical value is greater than 0.1, the correlation between pixels is obvious. In this case, if the HNR is used, obvious grid problems occur when the ISO is high. Please confirm with the sensor manufacturer whether DPC or other denoising operations are enabled inside the sensor. These algorithms must be disabled.