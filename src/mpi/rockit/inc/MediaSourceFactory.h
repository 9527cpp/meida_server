/*
 * MediaSourceFactory.h
 *
 *  Created on: 2016年3月17日
 *      Author: terry
 */

#ifndef MEDIASOURCEFACTORY_H_
#define MEDIASOURCEFACTORY_H_

#include "MediaSource.h"

namespace av
{

class MediaSourceFactory
{
public:
	virtual ~MediaSourceFactory() {}

	virtual MediaSource* create(const std::string& url, const std::string& params) =0;

	virtual void destroy(MediaSource* source) =0;

};


typedef std::shared_ptr< MediaSourceFactory >		MediaSourceFactoryPtr;


struct MediaSourceDeleter
{
	explicit MediaSourceDeleter(MediaSourceFactoryPtr& factory):
		m_factory(factory)
	{
	}

	void operator ()(MediaSource* source)
	{
		if (m_factory)
		{
			m_factory->destroy(source);
		}
	}

	MediaSourceFactoryPtr	m_factory;

};




} /* namespace av */

#endif /* MEDIASOURCEFACTORY_H_ */
