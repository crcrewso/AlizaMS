#ifndef A_SPECTROSCOPYUTILS_H
#define A_SPECTROSCOPYUTILS_H

#include <mdcmDataSet.h>
#include <vector>
#include <QString>

class SpectroscopyData;
class ImageVariant;
class GLWidget;
class QProgressDialog;

class SpectroscopyUtils
{
public:
	static bool Read(
		const mdcm::DataSet&,
		SpectroscopyData*);
	static QString ProcessData(
		const mdcm::DataSet&,
		std::vector<ImageVariant*> &,
		int, GLWidget*, bool,
		QProgressDialog*,
		float);
};

#endif

