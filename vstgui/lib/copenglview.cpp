//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.1
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "copenglview.h"
#include "platform/iplatformframe.h"
#include "cframe.h"
#include <assert.h>

#if VSTGUI_OPENGL_SUPPORT

namespace VSTGUI {

//-----------------------------------------------------------------------------
COpenGLView::COpenGLView (const CRect& size)
: CView (size)
, platformOpenGLView (0)
{
}

//-----------------------------------------------------------------------------
COpenGLView::~COpenGLView ()
{
	assert (platformOpenGLView == 0);
}

//-----------------------------------------------------------------------------
void COpenGLView::updatePlatformOpenGLViewSize ()
{
	if (platformOpenGLView)
	{
		CRect visibleSize (getVisibleSize ());
		CPoint offset;
		localToFrame (offset);
		visibleSize.offset (offset.x, offset.y);
		platformOpenGLView->viewSizeChanged (visibleSize);
		platformOpenGLView->invalidRect (getViewSize ());
	}
}

// CView
//-----------------------------------------------------------------------------
void COpenGLView::setViewSize (const CRect& rect, bool invalid)
{
	CView::setViewSize (rect, invalid);
	updatePlatformOpenGLViewSize ();
}

//-----------------------------------------------------------------------------
void COpenGLView::parentSizeChanged ()
{
	CView::parentSizeChanged ();
	updatePlatformOpenGLViewSize ();
}

//-----------------------------------------------------------------------------
bool COpenGLView::attached (CView* parent)
{
	if (CView::attached (parent))
	{
		IPlatformFrame* platformFrame = getFrame ()->getPlatformFrame ();
		platformOpenGLView = platformFrame ? platformFrame->createPlatformOpenGLView () : 0;
		if (platformOpenGLView)
		{
			if (platformOpenGLView->init (this, getPixelFormat ()))
			{
				updatePlatformOpenGLViewSize ();
				return true;
			}
			platformOpenGLView->forget ();
			platformOpenGLView = 0;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool COpenGLView::removed (CView* parent)
{
	if (platformOpenGLView)
	{
		platformOpenGLView->remove ();
		platformOpenGLView->forget ();
		platformOpenGLView = 0;
	}
	return CView::removed (parent);
}

//-----------------------------------------------------------------------------
void COpenGLView::invalidRect (const CRect& rect)
{
	if (platformOpenGLView)
	{
		platformOpenGLView->invalidRect (rect);
	}
	else
	{
		CView::invalidRect (rect);
	}
}

} // namespace

#endif // VSTGUI_OPENGL_SUPPORT