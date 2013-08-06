
#include "3dsFileFormat.h"
#include "3dsFileLoader.h"
#include "AppFrame.h"

#include "Window.h"
#include "ViewControl.h"

int main()

{
	CViewControl Control("Ciao");
	CAppFrame Finestra("Hello", 10, 10, 400, 400);
	Control.AppendWindow(&Finestra);
	Control.Start();
	return (0);
}
