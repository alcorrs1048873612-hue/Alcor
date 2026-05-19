// 以下代码添加到 Test1View.cpp 末尾
// 需要在 BEGIN_MESSAGE_MAP 中添加对应的消息映射：
//   ON_COMMAND(ID_BUTTERWORTH_LPF, &CTest1View::OnButterworthLPF)
//   ON_COMMAND(ID_BUTTERWORTH_HPF, &CTest1View::OnButterworthHPF)
//
// 需要在 Test1View.h 中声明：
//   afx_msg void OnButterworthLPF();
//   afx_msg void OnButterworthHPF();
//
// 需要在 Resource.h 中定义：
//   #define ID_BUTTERWORTH_LPF    32784
//   #define ID_BUTTERWORTH_HPF    32785
//
// 需要在文件顶部添加：
//   #include <math.h>


//=============================================================
// 巴特沃斯低通滤波器（频域滤波）
// 传递函数 H(u,v) = 1 / (1 + (D(u,v)/D0)^(2n))
// D(u,v) 为频率域中点(u,v)到中心的距离
// D0 为截止频率，n 为阶数
// 
// 实现步骤：
// 1. 对图像做二维DFT（离散傅里叶变换）
// 2. 将频谱中心移到图像中央
// 3. 乘以巴特沃斯低通传递函数
// 4. 反中心化
// 5. 做二维IDFT（逆变换）
// 6. 取实部作为输出图像
//=============================================================
void CTest1View::OnButterworthLPF()
{
	if (pInfo == NULL || pImage == NULL)
	{
		AfxMessageBox(_T("图像未打开，请先打开一幅图像！"));
		return;
	}

	if (pInfo->bmiHeader.biBitCount != 8)
	{
		AfxMessageBox(_T("当前巴特沃斯低通滤波只处理8位灰度图像！"));
		return;
	}

	int lWidth = pInfo->bmiHeader.biWidth;
	int lHeight = pInfo->bmiHeader.biHeight;
	if (lHeight < 0) lHeight = -lHeight;

	int imgSize = lWidth * lHeight;

	// 巴特沃斯滤波器参数
	double D0 = 60.0;    // 截止频率（可根据需要调整）
	int n = 2;           // 阶数（2阶比较常用）

	// 分配频域数据的实部和虚部
	// 用double数组存储，因为傅里叶变换涉及浮点运算
	double* fReal = new double[imgSize];    // 实部
	double* fImag = new double[imgSize];    // 虚部
	double* FReal = new double[imgSize];    // 变换后实部
	double* FImag = new double[imgSize];    // 变换后虚部

	int i, j;

	// 第一步：将图像数据复制到实部，虚部置0
	// 同时乘以(-1)^(i+j)实现频谱中心化（等效于变换后移频）
	for (j = 0; j < lHeight; j++)
	{
		for (i = 0; i < lWidth; i++)
		{
			double pixel = (double)pImage[j * lWidth + i];

			// 乘以(-1)^(i+j)，使频谱中心化
			if ((i + j) % 2 == 1)
				pixel = -pixel;

			fReal[j * lWidth + i] = pixel;
			fImag[j * lWidth + i] = 0.0;
		}
	}

	// 第二步：逐行做一维DFT
	double* tempReal = new double[lWidth > lHeight ? lWidth : lHeight];
	double* tempImag = new double[lWidth > lHeight ? lWidth : lHeight];

	// 对每一行做一维DFT
	for (j = 0; j < lHeight; j++)
	{
		// 对第j行做DFT
		for (i = 0; i < lWidth; i++)
		{
			double sumReal = 0.0;
			double sumImag = 0.0;

			for (int k = 0; k < lWidth; k++)
			{
				double angle = -2.0 * 3.14159265358979 * i * k / lWidth;
				sumReal += fReal[j * lWidth + k] * cos(angle) - fImag[j * lWidth + k] * sin(angle);
				sumImag += fReal[j * lWidth + k] * sin(angle) + fImag[j * lWidth + k] * cos(angle);
			}

			tempReal[i] = sumReal;
			tempImag[i] = sumImag;
		}

		// 写回
		for (i = 0; i < lWidth; i++)
		{
			fReal[j * lWidth + i] = tempReal[i];
			fImag[j * lWidth + i] = tempImag[i];
		}
	}

	// 对每一列做一维DFT
	for (i = 0; i < lWidth; i++)
	{
		for (j = 0; j < lHeight; j++)
		{
			double sumReal = 0.0;
			double sumImag = 0.0;

			for (int k = 0; k < lHeight; k++)
			{
				double angle = -2.0 * 3.14159265358979 * j * k / lHeight;
				sumReal += fReal[k * lWidth + i] * cos(angle) - fImag[k * lWidth + i] * sin(angle);
				sumImag += fReal[k * lWidth + i] * sin(angle) + fImag[k * lWidth + i] * cos(angle);
			}

			tempReal[j] = sumReal;
			tempImag[j] = sumImag;
		}

		// 写回
		for (j = 0; j < lHeight; j++)
		{
			FReal[j * lWidth + i] = tempReal[j];
			FImag[j * lWidth + i] = tempImag[j];
		}
	}

	// 第三步：在频域乘以巴特沃斯低通传递函数
	// H(u,v) = 1 / (1 + (D(u,v)/D0)^(2n))
	double centerU = lHeight / 2.0;
	double centerV = lWidth / 2.0;

	for (j = 0; j < lHeight; j++)
	{
		for (i = 0; i < lWidth; i++)
		{
			// 计算点(j,i)到频域中心的距离
			double dist = sqrt((j - centerU) * (j - centerU) + (i - centerV) * (i - centerV));

			// 巴特沃斯低通传递函数
			double H = 1.0 / (1.0 + pow(dist / D0, 2.0 * n));

			// 频域相乘
			FReal[j * lWidth + i] *= H;
			FImag[j * lWidth + i] *= H;
		}
	}

	// 第四步：做二维IDFT（逆傅里叶变换）
	// 先对每一列做一维IDFT
	for (i = 0; i < lWidth; i++)
	{
		for (j = 0; j < lHeight; j++)
		{
			double sumReal = 0.0;
			double sumImag = 0.0;

			for (int k = 0; k < lHeight; k++)
			{
				double angle = 2.0 * 3.14159265358979 * j * k / lHeight;
				sumReal += FReal[k * lWidth + i] * cos(angle) - FImag[k * lWidth + i] * sin(angle);
				sumImag += FReal[k * lWidth + i] * sin(angle) + FImag[k * lWidth + i] * cos(angle);
			}

			tempReal[j] = sumReal / lHeight;
			tempImag[j] = sumImag / lHeight;
		}

		for (j = 0; j < lHeight; j++)
		{
			fReal[j * lWidth + i] = tempReal[j];
			fImag[j * lWidth + i] = tempImag[j];
		}
	}

	// 再对每一行做一维IDFT
	for (j = 0; j < lHeight; j++)
	{
		for (i = 0; i < lWidth; i++)
		{
			double sumReal = 0.0;
			double sumImag = 0.0;

			for (int k = 0; k < lWidth; k++)
			{
				double angle = 2.0 * 3.14159265358979 * i * k / lWidth;
				sumReal += fReal[j * lWidth + k] * cos(angle) - fImag[j * lWidth + k] * sin(angle);
				sumImag += fReal[j * lWidth + k] * sin(angle) + fImag[j * lWidth + k] * cos(angle);
			}

			tempReal[i] = sumReal / lWidth;
			tempImag[i] = sumImag / lWidth;
		}

		for (i = 0; i < lWidth; i++)
		{
			fReal[j * lWidth + i] = tempReal[i];
			fImag[j * lWidth + i] = tempImag[i];
		}
	}

	// 第五步：取实部，乘以(-1)^(i+j)还原，转为图像
	BYTE* pNewImage = new BYTE[imgSize];

	for (j = 0; j < lHeight; j++)
	{
		for (i = 0; i < lWidth; i++)
		{
			double val = fReal[j * lWidth + i];

			// 还原中心化的(-1)^(i+j)
			if ((i + j) % 2 == 1)
				val = -val;

			// 四舍五入并截断到[0,255]
			int pixel = (int)(val + 0.5);
			if (pixel < 0) pixel = 0;
			if (pixel > 255) pixel = 255;

			pNewImage[j * lWidth + i] = (BYTE)pixel;
		}
	}

	// 释放临时内存
	delete[] fReal;
	delete[] fImag;
	delete[] FReal;
	delete[] FImag;
	delete[] tempReal;
	delete[] tempImag;

	// 替换原图
	delete[] pImage;
	pImage = pNewImage;

	CString msg;
	msg.Format(_T("巴特沃斯低通滤波完成！截止频率D0=%.0f，阶数n=%d"), D0, n);
	AfxMessageBox(msg);

	Invalidate(TRUE);
}


//=============================================================
// 巴特沃斯高通滤波器（频域滤波）
// 传递函数 H(u,v) = 1 / (1 + (D0/D(u,v))^(2n))
// 高通 = 1 - 低通，或者直接用上面公式
// D(u,v) 为频率域中点(u,v)到中心的距离
// D0 为截止频率，n 为阶数
//
// 实现步骤与低通相同，只是传递函数不同
// 先显示频域滤波后的结果（边缘增强效果）
//=============================================================
void CTest1View::OnButterworthHPF()
{
	if (pInfo == NULL || pImage == NULL)
	{
		AfxMessageBox(_T("图像未打开，请先打开一幅图像！"));
		return;
	}

	if (pInfo->bmiHeader.biBitCount != 8)
	{
		AfxMessageBox(_T("当前巴特沃斯高通滤波只处理8位灰度图像！"));
		return;
	}

	int lWidth = pInfo->bmiHeader.biWidth;
	int lHeight = pInfo->bmiHeader.biHeight;
	if (lHeight < 0) lHeight = -lHeight;

	int imgSize = lWidth * lHeight;

	// 巴特沃斯滤波器参数
	double D0 = 30.0;    // 截止频率（高通一般比低通小一些）
	int n = 2;           // 阶数

	// 分配频域数据
	double* fReal = new double[imgSize];
	double* fImag = new double[imgSize];
	double* FReal = new double[imgSize];
	double* FImag = new double[imgSize];

	int i, j;

	// 第一步：图像数据加载到实部，乘以(-1)^(i+j)中心化
	for (j = 0; j < lHeight; j++)
	{
		for (i = 0; i < lWidth; i++)
		{
			double pixel = (double)pImage[j * lWidth + i];

			if ((i + j) % 2 == 1)
				pixel = -pixel;

			fReal[j * lWidth + i] = pixel;
			fImag[j * lWidth + i] = 0.0;
		}
	}

	// 第二步：二维DFT（按行列分离）
	double* tempReal = new double[lWidth > lHeight ? lWidth : lHeight];
	double* tempImag = new double[lWidth > lHeight ? lWidth : lHeight];

	// 对每一行做DFT
	for (j = 0; j < lHeight; j++)
	{
		for (i = 0; i < lWidth; i++)
		{
			double sumReal = 0.0;
			double sumImag = 0.0;

			for (int k = 0; k < lWidth; k++)
			{
				double angle = -2.0 * 3.14159265358979 * i * k / lWidth;
				sumReal += fReal[j * lWidth + k] * cos(angle) - fImag[j * lWidth + k] * sin(angle);
				sumImag += fReal[j * lWidth + k] * sin(angle) + fImag[j * lWidth + k] * cos(angle);
			}

			tempReal[i] = sumReal;
			tempImag[i] = sumImag;
		}

		for (i = 0; i < lWidth; i++)
		{
			fReal[j * lWidth + i] = tempReal[i];
			fImag[j * lWidth + i] = tempImag[i];
		}
	}

	// 对每一列做DFT
	for (i = 0; i < lWidth; i++)
	{
		for (j = 0; j < lHeight; j++)
		{
			double sumReal = 0.0;
			double sumImag = 0.0;

			for (int k = 0; k < lHeight; k++)
			{
				double angle = -2.0 * 3.14159265358979 * j * k / lHeight;
				sumReal += fReal[k * lWidth + i] * cos(angle) - fImag[k * lWidth + i] * sin(angle);
				sumImag += fReal[k * lWidth + i] * sin(angle) + fImag[k * lWidth + i] * cos(angle);
			}

			tempReal[j] = sumReal;
			tempImag[j] = sumImag;
		}

		for (j = 0; j < lHeight; j++)
		{
			FReal[j * lWidth + i] = tempReal[j];
			FImag[j * lWidth + i] = tempImag[j];
		}
	}

	// 第三步：在频域乘以巴特沃斯高通传递函数
	// H(u,v) = 1 / (1 + (D0/D(u,v))^(2n))
	// 注意：D(u,v)=0时H=0（直流分量被滤掉）
	double centerU = lHeight / 2.0;
	double centerV = lWidth / 2.0;

	for (j = 0; j < lHeight; j++)
	{
		for (i = 0; i < lWidth; i++)
		{
			double dist = sqrt((j - centerU) * (j - centerU) + (i - centerV) * (i - centerV));

			double H;
			if (dist < 1e-6)
			{
				// 中心点距离为0，高通传递函数H=0（完全滤除直流分量）
				H = 0.0;
			}
			else
			{
				// 巴特沃斯高通传递函数
				H = 1.0 / (1.0 + pow(D0 / dist, 2.0 * n));
			}

			FReal[j * lWidth + i] *= H;
			FImag[j * lWidth + i] *= H;
		}
	}

	// 第四步：二维IDFT
	// 对每一列做IDFT
	for (i = 0; i < lWidth; i++)
	{
		for (j = 0; j < lHeight; j++)
		{
			double sumReal = 0.0;
			double sumImag = 0.0;

			for (int k = 0; k < lHeight; k++)
			{
				double angle = 2.0 * 3.14159265358979 * j * k / lHeight;
				sumReal += FReal[k * lWidth + i] * cos(angle) - FImag[k * lWidth + i] * sin(angle);
				sumImag += FReal[k * lWidth + i] * sin(angle) + FImag[k * lWidth + i] * cos(angle);
			}

			tempReal[j] = sumReal / lHeight;
			tempImag[j] = sumImag / lHeight;
		}

		for (j = 0; j < lHeight; j++)
		{
			fReal[j * lWidth + i] = tempReal[j];
			fImag[j * lWidth + i] = tempImag[j];
		}
	}

	// 对每一行做IDFT
	for (j = 0; j < lHeight; j++)
	{
		for (i = 0; i < lWidth; i++)
		{
			double sumReal = 0.0;
			double sumImag = 0.0;

			for (int k = 0; k < lWidth; k++)
			{
				double angle = 2.0 * 3.14159265358979 * i * k / lWidth;
				sumReal += fReal[j * lWidth + k] * cos(angle) - fImag[j * lWidth + k] * sin(angle);
				sumImag += fReal[j * lWidth + k] * sin(angle) + fImag[j * lWidth + k] * cos(angle);
			}

			tempReal[i] = sumReal / lWidth;
			tempImag[i] = sumImag / lWidth;
		}

		for (i = 0; i < lWidth; i++)
		{
			fReal[j * lWidth + i] = tempReal[i];
			fImag[j * lWidth + i] = tempImag[i];
		}
	}

	// 第五步：取实部还原图像
	BYTE* pNewImage = new BYTE[imgSize];

	// 高通滤波后图像可能有负值（直流被去掉了），需要做灰度拉伸
	// 先找最大最小值
	double minVal = fReal[0];
	double maxVal = fReal[0];

	for (j = 0; j < lHeight; j++)
	{
		for (i = 0; i < lWidth; i++)
		{
			double val = fReal[j * lWidth + i];
			if ((i + j) % 2 == 1)
				val = -val;

			if (val < minVal) minVal = val;
			if (val > maxVal) maxVal = val;
		}
	}

	// 线性拉伸到[0, 255]
	double range = maxVal - minVal;
	if (range < 1e-6) range = 1.0;   // 防止除以0

	for (j = 0; j < lHeight; j++)
	{
		for (i = 0; i < lWidth; i++)
		{
			double val = fReal[j * lWidth + i];

			// 还原中心化
			if ((i + j) % 2 == 1)
				val = -val;

			// 线性拉伸到[0,255]
			int pixel = (int)((val - minVal) / range * 255.0 + 0.5);
			if (pixel < 0) pixel = 0;
			if (pixel > 255) pixel = 255;

			pNewImage[j * lWidth + i] = (BYTE)pixel;
		}
	}

	// 释放临时内存
	delete[] fReal;
	delete[] fImag;
	delete[] FReal;
	delete[] FImag;
	delete[] tempReal;
	delete[] tempImag;

	// 替换原图
	delete[] pImage;
	pImage = pNewImage;

	CString msg;
	msg.Format(_T("巴特沃斯高通滤波完成！截止频率D0=%.0f，阶数n=%d"), D0, n);
	AfxMessageBox(msg);

	Invalidate(TRUE);
}
