void GameFrame(game_memory* Memory, game_input* Input)
{
	if(Input->Up)
	{
		Memory->red = 255;
		Memory->green = 0;
		Memory->blue = 0;
	}
	else if(Input->Down)
	{
		Memory->red = 0;
		Memory->green = 255;
		Memory->blue = 0;
	}
	else if(Input->Left)
	{
		Memory->red = 0;
		Memory->green = 0;
		Memory->blue = 255;
	}
	else if(Input->Right)
	{
		Memory->red = 255;
		Memory->green = 255;
		Memory->blue = 255;
	}
}
