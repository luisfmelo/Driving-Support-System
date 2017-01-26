
void refreshOUTvalues()
{
	if (Out_L01<L01)
	{
		Out_L01++;
	}
	else
	{
		if (Out_L01>0)
		{
			Out_L01--;
		} 
	}
	
	if ((Out_L02<L02)||(0==Out_L02))
	{
		Out_L02++;
	}
	else
	{
		if (Out_L02>0)
		{
			Out_L02--;
		}
	}
	
	if ((Out_L03<L03)||(1==Out_L03))
	{
		Out_L03++;
	}
	else
	{
		if (Out_L03>0)
		{
			Out_L03--;
		}
	}
	
	if ((Out_L04<L04)||(1==Out_L04))
	{
		Out_L04++;
	}
	else
	{
		if (Out_L04>0)
		{
			Out_L04--;
		}
	}
	
	if ((Out_L05<L05)||(1==Out_L05))
	{
		Out_L05++;
	}
	else
	{
		if (Out_L05>0)
		{
			Out_L05--;
		}
	}
	
	if ((Out_LLOW<LLOW)||(1==Out_LLOW))
	{
		Out_LLOW++;
	}
	else
	{
		if (Out_LLOW>0)
		{
			Out_LLOW--;
		}
	}
	
	if (Out_R01<R01)
	{
		Out_R01++;
	}
	else
	{
		if (Out_R01>0)
		{
			Out_R01--;
		} 
	}
	
	if ((Out_R02<R02)||(0==Out_R02))
	{
		Out_R02++;
	}
	else
	{
		if (Out_R02>0)
		{
			Out_R02--;
		}
	}
	
	if ((Out_R03<R03)||(1==Out_R03))
	{
		Out_R03++;
	}
	else
	{
		if (Out_R03>0)
		{
			Out_R03--;
		}
	}
	
	if ((Out_R04<R04)||(1==Out_R04))
	{
		Out_R04++;
	}
	else
	{
		if (Out_R04>0)
		{
			Out_R04--;
		}
	}
	
	Out_R05=R05;
/*	if ((Out_R05<R05)||(1==Out_R05))
	{
		Out_R05++;
	}
	else
	{
		if (Out_R05>0)
		{
			Out_R05--;
		}
	}
*/	
	if ((Out_RLOW<RLOW)||(1==Out_RLOW))
	{
		Out_RLOW++;
	}
	else
	{
		if (Out_RLOW>0)
		{
			Out_RLOW--;
		}
	}
	
}
