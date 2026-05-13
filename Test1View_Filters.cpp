// 以下代码添加到 Test1View.cpp 末尾
// 需要在 BEGIN_MESSAGE_MAP 中添加对应的消息映射：
//   ON_COMMAND(ID_MEAN_FILTER, &CTest1View::OnMeanFilter)
//   ON_COMMAND(ID_MEDIAN_FILTER, &CTest1View::OnMedianFilter)
//   ON_COMMAND(ID_LAPLACIAN_SHARPEN, &CTest1View::OnLaplacianSharpen)
//   ON_COMMAND(ID_SOBEL_SHARPEN, &CTest1View::OnSobelSharpen)
//
// 需要在 Test1View.h 中声明：
//   afx_msg void OnMeanFilter();
//   afx_msg void OnMedianFilter();
//   afx_msg void OnLaplacianSharpen();
//   afx_msg void OnSobelSharpen();


//=============================================================
// 领域平均滤波（均值滤波）—— 3x3模板
//=============================================================
void CTest1View::OnMeanFilter()
{
	if (pInfo == NULL || pImage == NULL)
	{
		AfxMessageBox(_T("图像未打开，请先打开一幅图像！"));
		return;
	}

	if (pInfo->bmiHeader.biBitCount != 8)
	{
		AfxMessageBox(_T("当前均值滤波只处理8位灰度图像！"));
		return;
	}

	int lWidth = pInfo->bmiHeader.biWidth;
	int lHeight = pInfo->bmiHeader.biHeight;
	if (lHeight < 0) lHeight = -lHeight;

	// 分配新图像内存
	BYTE* pNewImage = new BYTE[lWidth * lHeight];
	memset(pNewImage, 0, lWidth * lHeight);

	// 3x3均值滤波模板
	// 对图像除边界外的每个像素进行3x3邻域平均
	int i, j;
	int sum = 0;

	for (j = 1; j < lHeight - 1; j++)
	{
		for (i = 1; i < lWidth - 1; i++)
		{
			// 计算3x3邻域的灰度值之和
			sum = 0;
			sum += (int)pImage[(j - 1) * lWidth + (i - 1)];
			sum += (int)pImage[(j - 1) * lWidth + i];
			sum += (int)pImage[(j - 1) * lWidth + (i + 1)];
			sum += (int)pImage[j * lWidth + (i - 1)];
			sum += (int)pImage[j * lWidth + i];
			sum += (int)pImage[j * lWidth + (i + 1)];
			sum += (int)pImage[(j + 1) * lWidth + (i - 1)];
			sum += (int)pImage[(j + 1) * lWidth + i];
			sum += (int)pImage[(j + 1) * lWidth + (i + 1)];

			// 求平均值
			int avg = sum / 9;

			// 防止越界
			if (avg < 0) avg = 0;
			if (avg > 255) avg = 255;

			pNewImage[j * lWidth + i] = (BYTE)avg;
		}
	}

	// 处理边界像素，直接复制原图
	for (i = 0; i < lWidth; i++)
	{
		pNewImage[0 * lWidth + i] = pImage[0 * lWidth + i];                    // 第一行
		pNewImage[(lHeight - 1) * lWidth + i] = pImage[(lHeight - 1) * lWidth + i]; // 最后一行
	}
	for (j = 0; j < lHeight; j++)
	{
		pNewImage[j * lWidth + 0] = pImage[j * lWidth + 0];                    // 第一列
		pNewImage[j * lWidth + (lWidth - 1)] = pImage[j * lWidth + (lWidth - 1)]; // 最后一列
	}

	// 替换原图
	delete[] pImage;
	pImage = pNewImage;

	Invalidate(TRUE);
}


//=============================================================
// 中值滤波 —— 3x3窗口
//=============================================================
void CTest1View::OnMedianFilter()
{
	if (pInfo == NULL || pImage == NULL)
	{
		AfxMessageBox(_T("图像未打开，请先打开一幅图像！"));
		return;
	}

	if (pInfo->bmiHeader.biBitCount != 8)
	{
		AfxMessageBox(_T("当前中值滤波只处理8位灰度图像！"));
		return;
	}

	int lWidth = pInfo->bmiHeader.biWidth;
	int lHeight = pInfo->bmiHeader.biHeight;
	if (lHeight < 0) lHeight = -lHeight;

	// 分配新图像内存
	BYTE* pNewImage = new BYTE[lWidth * lHeight];
	memset(pNewImage, 0, lWidth * lHeight);

	int i, j, k, m;

	// 对图像除边界外的每个像素进行3x3中值滤波
	for (j = 1; j < lHeight - 1; j++)
	{
		for (i = 1; i < lWidth - 1; i++)
		{
			// 取3x3邻域的9个像素值
			int window[9];
			int idx = 0;

			for (m = -1; m <= 1; m++)
			{
				for (k = -1; k <= 1; k++)
				{
					window[idx] = (int)pImage[(j + m) * lWidth + (i + k)];
					idx++;
				}
			}

			// 冒泡排序，找中值
			int temp;
			for (m = 0; m < 8; m++)
			{
				for (k = 0; k < 8 - m; k++)
				{
					if (window[k] > window[k + 1])
					{
						temp = window[k];
						window[k] = window[k + 1];
						window[k + 1] = temp;
					}
				}
			}

			// 取中间值作为输出
			int median = window[4];

			// 防止越界
			if (median < 0) median = 0;
			if (median > 255) median = 255;

			pNewImage[j * lWidth + i] = (BYTE)median;
		}
	}

	// 处理边界像素，直接复制原图
	for (i = 0; i < lWidth; i++)
	{
		pNewImage[0 * lWidth + i] = pImage[0 * lWidth + i];
		pNewImage[(lHeight - 1) * lWidth + i] = pImage[(lHeight - 1) * lWidth + i];
	}
	for (j = 0; j < lHeight; j++)
	{
		pNewImage[j * lWidth + 0] = pImage[j * lWidth + 0];
		pNewImage[j * lWidth + (lWidth - 1)] = pImage[j * lWidth + (lWidth - 1)];
	}

	// 替换原图
	delete[] pImage;
	pImage = pNewImage;

	Invalidate(TRUE);
}


//=============================================================
// 线性锐化滤波（拉普拉斯算子）—— 同时输出梯度图像
// 使用拉普拉斯模板：
//     0  -1   0
//    -1   4  -1
//     0  -1   0
// 锐化结果 = 原图 + 拉普拉斯梯度
//=============================================================
void CTest1View::OnLaplacianSharpen()
{
	if (pInfo == NULL || pImage == NULL)
	{
		AfxMessageBox(_T("图像未打开，请先打开一幅图像！"));
		return;
	}

	if (pInfo->bmiHeader.biBitCount != 8)
	{
		AfxMessageBox(_T("当前拉普拉斯锐化只处理8位灰度图像！"));
		return;
	}

	int lWidth = pInfo->bmiHeader.biWidth;
	int lHeight = pInfo->bmiHeader.biHeight;
	if (lHeight < 0) lHeight = -lHeight;

	int imgSize = lWidth * lHeight;

	// 分配锐化结果图像
	BYTE* pSharpen = new BYTE[imgSize];
	memset(pSharpen, 0, imgSize);

	// 分配梯度图像（拉普拉斯响应的绝对值图）
	BYTE* pGradient = new BYTE[imgSize];
	memset(pGradient, 0, imgSize);

	int i, j;
	int laplacian = 0;
	int sharpenVal = 0;

	// 拉普拉斯算子卷积
	for (j = 1; j < lHeight - 1; j++)
	{
		for (i = 1; i < lWidth - 1; i++)
		{
			// 拉普拉斯模板计算
			//     0  -1   0
			//    -1   4  -1
			//     0  -1   0
			laplacian = 4 * (int)pImage[j * lWidth + i]
				- (int)pImage[(j - 1) * lWidth + i]
				- (int)pImage[(j + 1) * lWidth + i]
				- (int)pImage[j * lWidth + (i - 1)]
				- (int)pImage[j * lWidth + (i + 1)];

			// 梯度图像取绝对值
			int gradVal = laplacian;
			if (gradVal < 0) gradVal = -gradVal;
			if (gradVal > 255) gradVal = 255;
			pGradient[j * lWidth + i] = (BYTE)gradVal;

			// 锐化 = 原图 + 拉普拉斯
			sharpenVal = (int)pImage[j * lWidth + i] + laplacian;

			// 防止越界
			if (sharpenVal < 0) sharpenVal = 0;
			if (sharpenVal > 255) sharpenVal = 255;

			pSharpen[j * lWidth + i] = (BYTE)sharpenVal;
		}
	}

	// 边界像素直接复制原图
	for (i = 0; i < lWidth; i++)
	{
		pSharpen[0 * lWidth + i] = pImage[0 * lWidth + i];
		pSharpen[(lHeight - 1) * lWidth + i] = pImage[(lHeight - 1) * lWidth + i];
		pGradient[0 * lWidth + i] = 0;
		pGradient[(lHeight - 1) * lWidth + i] = 0;
	}
	for (j = 0; j < lHeight; j++)
	{
		pSharpen[j * lWidth + 0] = pImage[j * lWidth + 0];
		pSharpen[j * lWidth + (lWidth - 1)] = pImage[j * lWidth + (lWidth - 1)];
		pGradient[j * lWidth + 0] = 0;
		pGradient[j * lWidth + (lWidth - 1)] = 0;
	}

	// 先弹出一个对话框，把梯度图像用直方图窗口的方式展示出来
	// 这里我们用MessageBox简单提示，然后把梯度图像显示到主窗口
	// 为了同时输出梯度图像，我们先把梯度图保存为BMP文件
	// 保存梯度图像到文件
	CString strGradFile;
	strGradFile = _T("LaplacianGradient.bmp");

	CFile gradFile;
	if (gradFile.Open(strGradFile, CFile::modeCreate | CFile::modeWrite))
	{
		// 构造BMP文件头
		BITMAPFILEHEADER bfh;
		memset(&bfh, 0, sizeof(BITMAPFILEHEADER));
		bfh.bfType = 0x4D42;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
		bfh.bfSize = bfh.bfOffBits + imgSize;

		// 构造BMP信息头
		BITMAPINFOHEADER bih;
		memset(&bih, 0, sizeof(BITMAPINFOHEADER));
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = lWidth;
		bih.biHeight = lHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 8;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = imgSize;

		// 灰度调色板
		RGBQUAD palette[256];
		for (int n = 0; n < 256; n++)
		{
			palette[n].rgbBlue = n;
			palette[n].rgbGreen = n;
			palette[n].rgbRed = n;
			palette[n].rgbReserved = 0;
		}

		gradFile.Write(&bfh, sizeof(BITMAPFILEHEADER));
		gradFile.Write(&bih, sizeof(BITMAPINFOHEADER));
		gradFile.Write(palette, 256 * sizeof(RGBQUAD));
		gradFile.Write(pGradient, imgSize);
		gradFile.Close();

		CString msg;
		msg.Format(_T("拉普拉斯梯度图像已保存为：%s"), strGradFile);
		AfxMessageBox(msg);
	}
	else
	{
		AfxMessageBox(_T("梯度图像保存失败！"));
	}

	// 释放梯度图像内存
	delete[] pGradient;

	// 将锐化结果替换原图，显示锐化后的图像
	delete[] pImage;
	pImage = pSharpen;

	Invalidate(TRUE);
}


//=============================================================
// Sobel算子实现非线性锐化滤波 —— 同时输出梯度图像
// Sobel水平模板Gx：         Sobel垂直模板Gy：
//    -1   0   1                -1  -2  -1
//    -2   0   2                 0   0   0
//    -1   0   1                 1   2   1
// 梯度幅值 G = |Gx| + |Gy|
// 锐化结果 = 原图 + 梯度幅值
//=============================================================
void CTest1View::OnSobelSharpen()
{
	if (pInfo == NULL || pImage == NULL)
	{
		AfxMessageBox(_T("图像未打开，请先打开一幅图像！"));
		return;
	}

	if (pInfo->bmiHeader.biBitCount != 8)
	{
		AfxMessageBox(_T("当前Sobel锐化只处理8位灰度图像！"));
		return;
	}

	int lWidth = pInfo->bmiHeader.biWidth;
	int lHeight = pInfo->bmiHeader.biHeight;
	if (lHeight < 0) lHeight = -lHeight;

	int imgSize = lWidth * lHeight;

	// 分配锐化结果图像
	BYTE* pSharpen = new BYTE[imgSize];
	memset(pSharpen, 0, imgSize);

	// 分配梯度图像
	BYTE* pGradient = new BYTE[imgSize];
	memset(pGradient, 0, imgSize);

	int i, j;
	int Gx, Gy;
	int gradMag;
	int sharpenVal;

	// Sobel算子卷积
	for (j = 1; j < lHeight - 1; j++)
	{
		for (i = 1; i < lWidth - 1; i++)
		{
			// 取3x3邻域像素
			int p00 = (int)pImage[(j - 1) * lWidth + (i - 1)];
			int p01 = (int)pImage[(j - 1) * lWidth + i];
			int p02 = (int)pImage[(j - 1) * lWidth + (i + 1)];
			int p10 = (int)pImage[j * lWidth + (i - 1)];
			// int p11 = (int)pImage[j * lWidth + i];  // 中心点，Sobel不用
			int p12 = (int)pImage[j * lWidth + (i + 1)];
			int p20 = (int)pImage[(j + 1) * lWidth + (i - 1)];
			int p21 = (int)pImage[(j + 1) * lWidth + i];
			int p22 = (int)pImage[(j + 1) * lWidth + (i + 1)];

			// Sobel水平方向 Gx
			// -1  0  1
			// -2  0  2
			// -1  0  1
			Gx = (-1) * p00 + 0 * p01 + 1 * p02
				+ (-2) * p10 + 0 + 2 * p12
				+ (-1) * p20 + 0 * p21 + 1 * p22;

			// Sobel垂直方向 Gy
			// -1 -2 -1
			//  0  0  0
			//  1  2  1
			Gy = (-1) * p00 + (-2) * p01 + (-1) * p02
				+ 0 * p10 + 0 + 0 * p12
				+ 1 * p20 + 2 * p21 + 1 * p22;

			// 梯度幅值用绝对值之和近似
			gradMag = abs(Gx) + abs(Gy);

			// 梯度图像
			if (gradMag > 255) gradMag = 255;
			if (gradMag < 0) gradMag = 0;
			pGradient[j * lWidth + i] = (BYTE)gradMag;

			// 非线性锐化：原图 + 梯度幅值
			sharpenVal = (int)pImage[j * lWidth + i] + gradMag;

			// 防止越界
			if (sharpenVal > 255) sharpenVal = 255;
			if (sharpenVal < 0) sharpenVal = 0;

			pSharpen[j * lWidth + i] = (BYTE)sharpenVal;
		}
	}

	// 边界像素直接复制原图
	for (i = 0; i < lWidth; i++)
	{
		pSharpen[0 * lWidth + i] = pImage[0 * lWidth + i];
		pSharpen[(lHeight - 1) * lWidth + i] = pImage[(lHeight - 1) * lWidth + i];
		pGradient[0 * lWidth + i] = 0;
		pGradient[(lHeight - 1) * lWidth + i] = 0;
	}
	for (j = 0; j < lHeight; j++)
	{
		pSharpen[j * lWidth + 0] = pImage[j * lWidth + 0];
		pSharpen[j * lWidth + (lWidth - 1)] = pImage[j * lWidth + (lWidth - 1)];
		pGradient[j * lWidth + 0] = 0;
		pGradient[j * lWidth + (lWidth - 1)] = 0;
	}

	// 保存Sobel梯度图像为BMP文件
	CString strGradFile;
	strGradFile = _T("SobelGradient.bmp");

	CFile gradFile;
	if (gradFile.Open(strGradFile, CFile::modeCreate | CFile::modeWrite))
	{
		// 构造BMP文件头
		BITMAPFILEHEADER bfh;
		memset(&bfh, 0, sizeof(BITMAPFILEHEADER));
		bfh.bfType = 0x4D42;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
		bfh.bfSize = bfh.bfOffBits + imgSize;

		// 构造BMP信息头
		BITMAPINFOHEADER bih;
		memset(&bih, 0, sizeof(BITMAPINFOHEADER));
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = lWidth;
		bih.biHeight = lHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 8;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = imgSize;

		// 灰度调色板
		RGBQUAD palette[256];
		for (int n = 0; n < 256; n++)
		{
			palette[n].rgbBlue = n;
			palette[n].rgbGreen = n;
			palette[n].rgbRed = n;
			palette[n].rgbReserved = 0;
		}

		gradFile.Write(&bfh, sizeof(BITMAPFILEHEADER));
		gradFile.Write(&bih, sizeof(BITMAPINFOHEADER));
		gradFile.Write(palette, 256 * sizeof(RGBQUAD));
		gradFile.Write(pGradient, imgSize);
		gradFile.Close();

		CString msg;
		msg.Format(_T("Sobel梯度图像已保存为：%s"), strGradFile);
		AfxMessageBox(msg);
	}
	else
	{
		AfxMessageBox(_T("梯度图像保存失败！"));
	}

	// 释放梯度图内存
	delete[] pGradient;

	// 将锐化结果替换原图
	delete[] pImage;
	pImage = pSharpen;

	Invalidate(TRUE);
}
