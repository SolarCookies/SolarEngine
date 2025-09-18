#include "ContentWindow.h"

void ContentWindow::RenderContentWindow()
{
	ImGui::Begin("Content");
	ContentWindowSize = ImGui::GetWindowSize();

	int L;
	for(int i = 0; i < 100; ++i)
	{
		
		//DrawItem(i, std::to_string(i));
		if((ContentWindowSize.x / 15) * L >= ContentWindowSize.x)
		{
			//ImGui::NewLine();
			L = 0;
		}
		else {
			L++;
		}
	}

	ImGui::End();
}
