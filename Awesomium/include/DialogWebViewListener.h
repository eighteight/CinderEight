//
//  DialogWebViewListener.h
//  AwesomeSilk
//
//  Created by Gusev, Vladimir on 1/28/13.
//
//

#ifndef AwesomeSilk_DialogWebViewListener_h
#define AwesomeSilk_DialogWebViewListener_h
#include "WebViewListener.h"

class DialogWebViewListener : public Awesomium::WebViewListener::Dialog
{
    void OnShowFileChooser(Awesomium::WebView* caller,
                           const Awesomium::WebFileChooserInfo& chooser_info){
        
    }
    
    ///
    /// This event occurs when the page needs authentication from the user (for
    /// example, Basic HTTP Auth, NTLM Auth, etc). It is your responsibility to
    /// display a dialog so that users can input their username and password.
    /// This event is not modal.
    ///
    /// @see WebView::DidLogin
    /// @see WebView::DidCancelLogin
    ///
    void OnShowLoginDialog(Awesomium::WebView* caller,
                           const Awesomium::WebLoginDialogInfo& dialog_info){
    
    };
};


#endif
