import java.util.List;

import com.jogamp.opengl.GL2ES3;

public class OpenGLWheelTempGraph extends PositionedChart {

	final int dialResolution = 400; // how many quads to draw
	final float dialThickness = 0.4f; // percentage of the radius
	Samples samples;
	boolean breakAutoscaleMax;
	boolean breakAutoscaleMin;
	boolean wheelAutoscaleMin;
	boolean wheelAutoscaleMax;
	float wheelManualMin;
	float wheelManualMax;
	float breakManualMin;
	float breakManualMax;

	// plot region
	float xPlotLeft;
	float xPlotRight;
	float plotWidth;
	float yPlotTop;
	float yPlotBottom;
	float plotHeight;
	float yLabelBottom;
	float yLabelTop;
	float labelHeight;

	// min max labels
	boolean showMinMaxLabels;
	float yMinMaxLabelsBaseline;
	float yMinMaxLabelsTop;
	String minLabel;
	String maxLabel;
	float minLabelWidth;
	float maxLabelWidth;
	float xMinLabelLeft;
	float xMaxLabelLeft;
	float textHeight;
	float dataTextWidth;

	// reading label
	boolean showReadingLabel;
	boolean showBreakTemps;
	String wheelTopLabel;
	String wheelBottomLabel;
	String breakTopLabel;
	String breakBottomLabel;
	float readingLabelWidth;
	float xReadingLabelLeft;
	float yReadingLabelBaseline;
	float yReadingLabelTop;
	float readingLabelRadius;

	// dataset label
	boolean showLegend;
	String datasetLabel;
	float datasetLabelWidth;
	float yDatasetLabelBaseline;
	float yDatasetLabelTop;
	float xDatasetLabelLeft;
	float datasetLabelRadius;

	// constraints
	static final float BreakMinimumDefault = 160;
	static final float BreakMaximumDefault = 340;
	static final float TireMinimumDefault = 160; // NEED TO KNOW
	static final float TireMaximumDefault = 340; // NEED TO KNOW
	static final float LowerLimit = -Float.MAX_VALUE;
	static final float UpperLimit = Float.MAX_VALUE;

	static final int SampleCountDefault = 1000;
	static final int SampleCountLowerLimit = 1;
	static final int SampleCountUpperLimit = Integer.MAX_VALUE;

	// Data set amount bounds & variables
	final static int datasetAmountDefault = 1;
	final static int lowerDatasetAmount = 1;
	final static int upperDatasetAmount = 4;

	// Control Widgets
	WidgetDatasets wheelDatasetsWidget;
	WidgetDatasets breakDatasetsWidget;
	WidgetTextfieldsOptionalMinMax wheelMinMaxWidget;
	WidgetTextfieldsOptionalMinMax breakMinMaxWidget;
	WidgetTextfieldInteger sampleCountWidget;
	WidgetCheckbox showBreakTempsWidget;
	WidgetCheckbox showReadingLabelWidget;
	WidgetCheckbox showLegendWidget;
	WidgetCheckbox showMinMaxLabelsWidget;
	WidgetCheckbox showStatisticsWidget;
	
	// Datasets
	List<Dataset> wheelDatasets;
	List<Dataset> breakDatasets;

	public OpenGLWheelTempGraph(int x1, int y1, int x2, int y2) {
		super(x1, y1, x2, y2);
		
		wheelDatasetsWidget = new WidgetDatasets(4, new String[] { "Front Left", "Front Right", "Rear Left", "Rear Right" },
				newDatasets -> wheelDatasets = newDatasets);
		
		showBreakTempsWidget = new WidgetCheckbox("Show Break Temps", true,
				newShowReadingLabel -> showBreakTemps = newShowReadingLabel);
		
		breakDatasetsWidget = new WidgetDatasets(4, new String[] { "Front Left", "Front Right", "Rear Left", "Rear Right" },
				newDatasets -> breakDatasets = newDatasets);

		wheelMinMaxWidget = new WidgetTextfieldsOptionalMinMax("Tire Temp", TireMinimumDefault, TireMaximumDefault, LowerLimit,
				UpperLimit, (newAutoscaleMin, newManualMin) -> {
					wheelAutoscaleMin = newAutoscaleMin;
					wheelManualMin = newManualMin;
				}, (newAutoscaleMax, newManualMax) -> {
					wheelAutoscaleMax = newAutoscaleMax;
					wheelManualMax = newManualMax;
				});
		
		breakMinMaxWidget = new WidgetTextfieldsOptionalMinMax("Break Temp", BreakMinimumDefault, BreakMaximumDefault, LowerLimit,
				UpperLimit, (newAutoscaleMin, newManualMin) -> {
					breakAutoscaleMin = newAutoscaleMin;
					breakManualMin = newManualMin;
				}, (newAutoscaleMax, newManualMax) -> {
					breakAutoscaleMax = newAutoscaleMax;
					breakManualMax = newManualMax;
				});

		showReadingLabelWidget = new WidgetCheckbox("Show Reading Label", true,
				newShowReadingLabel -> showReadingLabel = newShowReadingLabel);

		widgets = new Widget[6];

		widgets[0] = wheelDatasetsWidget;
		widgets[1] = showBreakTempsWidget;
		widgets[2] = breakDatasetsWidget;
		widgets[3] = wheelMinMaxWidget;
		widgets[4] = breakMinMaxWidget;
		widgets[5] = showReadingLabelWidget;
	}

	@Override
	public EventHandler drawChart(GL2ES3 gl, float[] chartMatrix, int width, int height, int lastSampleNumber,
			double zoomLevel, int mouseX, int mouseY) {

		EventHandler handler = null;

		float[] wheelData = new float[4];
		float[] breakData = new float[4];

		float[] wheelTempMin = new float[4];
		float[] wheelTempMax = new float[4];
		
		float[] breakTempMin = new float[4];
		float[] breakTempMax = new float[4];

		// Setting first min/max
		for (int i = 0; i < 4; i++) {
			wheelTempMin[i] = wheelData[i];
			wheelTempMax[i] = wheelData[i];
			if (showBreakTemps) {
				breakTempMin[i] = breakData[i];
				breakTempMax[i] = breakData[i];
			}
		}
		
		// Setting real min max in the last 100 samples
		for (int i = 0; i < 4; i++) {
			for ( int j = lastSampleNumber - 100 > -1 ? lastSampleNumber - 100 : 0; j < lastSampleNumber - 1; j++) {
				wheelTempMin[i] = wheelDatasets.get(i).getSample(j) < wheelTempMin[i] ? wheelDatasets.get(i).getSample(j) : wheelTempMin[i];
				wheelTempMax[i] = wheelDatasets.get(i).getSample(j) > wheelTempMax[i] ? wheelDatasets.get(i).getSample(j) : wheelTempMax[i];
				if (showBreakTemps) {
					breakTempMin[i] = breakDatasets.get(i).getSample(j) < breakTempMin[i] ? breakDatasets.get(i).getSample(j) : breakTempMin[i];
					breakTempMax[i] = breakDatasets.get(i).getSample(j) > breakTempMax[i] ? breakDatasets.get(i).getSample(j) : breakTempMax[i];
				}
			}
		}
		
		// Deciding wether to use real or manual min/max
		for (int i = 0; i < 4; i++) {
			wheelTempMin[i] = wheelAutoscaleMin ? wheelTempMin[i] : wheelManualMin;
			wheelTempMax[i] = wheelAutoscaleMax ? wheelTempMax[i] : wheelManualMax;
			if (showBreakTemps) {
				breakTempMin[i] = breakAutoscaleMin ? breakTempMin[i] : breakManualMin;
				breakTempMax[i] = breakAutoscaleMax ? breakTempMax[i] : breakManualMax;
			}
		}
		
		// calculate x and y positions of everything
		xPlotLeft = Theme.tilePadding;
		xPlotRight = width - Theme.tilePadding;
		plotWidth = xPlotRight - xPlotLeft;
		yPlotTop = height - Theme.tilePadding;
		yPlotBottom = Theme.tilePadding;
		plotHeight = yPlotTop - yPlotBottom;
		
		
		if (showReadingLabel) {
			yLabelBottom = yPlotBottom;
			yPlotBottom += plotHeight / 4;
			yLabelTop = yPlotBottom;
			plotHeight = yPlotTop - yPlotBottom;
			labelHeight = yLabelTop - yLabelBottom;
		}
		
		boolean haveDatasets = wheelDatasets != null && !wheelDatasets.isEmpty() && lastSampleNumber > -1;
		
		if (haveDatasets) {
			for (int i = 0; i < 4; i++) {
				wheelData[i] = wheelDatasets.get(i).getSample(lastSampleNumber);
				if (showBreakTemps)
					breakData[i] = breakDatasets.get(i).getSample(lastSampleNumber);
			}
		}
		
		float[] xLeftWheel = new float[4];
		float[] yBottomWheel = new float[4];
		float[] xLeftBreak = new float[4];
		float[] yBottomBreak = new float[4];
		
		float graphHeightDif = 8.5f/10.0f;
		float graphWidthDif = 1.0f/4.0f;
		
		float wheelHeight = plotHeight / 3.0f;
		float wheelWidth = plotWidth / 5.0f;
		float breakHeight = wheelHeight * graphHeightDif;
		float breakWidth = wheelWidth * graphWidthDif;
		
		for (int i = 0; i < 4; i+=2) {
			xLeftWheel[i] = xPlotLeft + (wheelWidth - (wheelWidth / 3.0f));
			xLeftWheel[i+1] = xPlotRight - (wheelWidth - (wheelWidth / 3.0f)) - wheelWidth;
		}
		for (int i = 0; i < 2; i++) {
			yBottomWheel[i] = yPlotTop - (((plotHeight / 2) - wheelHeight) / 2.0f) - wheelHeight;
			yBottomWheel[i+2] = yPlotBottom + (((plotHeight / 2) - wheelHeight) / 2.0f);
		}
		if (showBreakTemps) {
			for (int i = 0; i < 4; i+=2) {
				xLeftBreak[i] = xLeftWheel[i] + wheelWidth + breakWidth;
				xLeftBreak[i+1] = xLeftWheel[i+1] - (breakWidth * 2);
			}
			for (int i = 0; i < 4; i++) {
				yBottomBreak[i] = yBottomWheel[i] + ((wheelHeight - breakHeight) / 2.0f);
			}
		}
		
		if (haveDatasets) {
			for (int i = 0; i < 4; i++) {
				OpenGL.drawQuad2D(gl, findTempColor(wheelTempMin[0], wheelTempMax[0], wheelData[i]), xLeftWheel[i], yBottomWheel[i], xLeftWheel[i] + wheelWidth,
						yBottomWheel[i] + wheelHeight);
				OpenGL.drawQuadOutline2D(gl, Theme.tickLinesColor, xLeftWheel[i], yBottomWheel[i], xLeftWheel[i] + wheelWidth,
						yBottomWheel[i] + wheelHeight);
				if (showBreakTemps) {
					OpenGL.drawQuad2D(gl, findTempColor(breakTempMin[0], breakTempMax[0], breakData[i]), xLeftBreak[i], yBottomBreak[i], xLeftBreak[i] + breakWidth,
							yBottomBreak[i] + breakHeight);
					OpenGL.drawQuadOutline2D(gl, Theme.tickLinesColor, xLeftBreak[i], yBottomBreak[i], xLeftBreak[i] + breakWidth,
							yBottomBreak[i] + breakHeight);
				}
			}
		}
		
		if (showReadingLabel && haveDatasets) {
			String dataUnit;
			if (wheelDatasets.get(0).unit.equals("Volts"))
				dataUnit = "C";
			else 
				dataUnit = wheelDatasets.get(0).unit;
			String[] posLabel = {"FL", "FR", "RL", "RR"};
			yReadingLabelBaseline = yLabelTop;
			String[] wheelLabels = {ChartUtils.formattedNumber(wheelData[0], 3) + dataUnit, ChartUtils.formattedNumber(wheelData[1], 3) + dataUnit, ChartUtils.formattedNumber(wheelData[2], 3) + dataUnit, ChartUtils.formattedNumber(wheelData[3], 3) + dataUnit};
			if (!showBreakTemps) {
				OpenGL.drawSmallText(gl, posLabel[0], (int) (xLeftWheel[0] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, posLabel[0]) / 2)), (int) yReadingLabelBaseline, 0);
				OpenGL.drawSmallText(gl, wheelLabels[0], (int) (xLeftWheel[0] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, wheelLabels[0]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight), 0);
				OpenGL.drawSmallText(gl, posLabel[2], (int) (xLeftWheel[0] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, posLabel[2]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 3), 0);
				OpenGL.drawSmallText(gl, wheelLabels[2], (int) (xLeftWheel[0] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, wheelLabels[2]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 4), 0);
				OpenGL.drawSmallText(gl, posLabel[1], (int) (xLeftWheel[1] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, posLabel[1]) / 2)), (int) yReadingLabelBaseline, 0);
				OpenGL.drawSmallText(gl, wheelLabels[1], (int) (xLeftWheel[1] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, wheelLabels[1]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight), 0);
				OpenGL.drawSmallText(gl, posLabel[3], (int) (xLeftWheel[1] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, posLabel[3]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 3), 0);
				OpenGL.drawSmallText(gl, wheelLabels[3], (int) (xLeftWheel[1] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, wheelLabels[3]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 4), 0);
			} else {
				String[] breakLabels = {ChartUtils.formattedNumber(breakData[0], 3) + dataUnit, ChartUtils.formattedNumber(breakData[1], 3) + dataUnit, ChartUtils.formattedNumber(breakData[2], 3) + dataUnit, ChartUtils.formattedNumber(breakData[3], 3) + dataUnit};
				OpenGL.drawSmallText(gl, "T", (int) (xLeftWheel[0] - (OpenGL.smallTextWidth(gl, "T") * 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight), 0);
				OpenGL.drawSmallText(gl, "B", (int) (xLeftWheel[0] - (OpenGL.smallTextWidth(gl, "T") * 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 2), 0);
				OpenGL.drawSmallText(gl, "T", (int) (xLeftWheel[1] + wheelWidth + (OpenGL.smallTextWidth(gl, "T") * 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight), 0);
				OpenGL.drawSmallText(gl, "B", (int) (xLeftWheel[1] + wheelWidth + (OpenGL.smallTextWidth(gl, "T") * 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 2), 0);
				
				OpenGL.drawSmallText(gl, "T", (int) (xLeftWheel[0] - (OpenGL.smallTextWidth(gl, "T") * 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 5), 0);
				OpenGL.drawSmallText(gl, "B", (int) (xLeftWheel[0] - (OpenGL.smallTextWidth(gl, "T") * 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 6), 0);
				OpenGL.drawSmallText(gl, "T", (int) (xLeftWheel[1] + wheelWidth + (OpenGL.smallTextWidth(gl, "T") * 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 5), 0);
				OpenGL.drawSmallText(gl, "B", (int) (xLeftWheel[1] + wheelWidth + (OpenGL.smallTextWidth(gl, "T") * 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 6), 0);
				
				OpenGL.drawSmallText(gl, posLabel[0], (int) (xLeftWheel[0] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, posLabel[0]) / 2)), (int) yReadingLabelBaseline, 0);
				OpenGL.drawSmallText(gl, wheelLabels[0], (int) (xLeftWheel[0] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, wheelLabels[0]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight), 0);
				OpenGL.drawSmallText(gl, breakLabels[0], (int) (xLeftWheel[0] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, breakLabels[0]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 2), 0);
				
				OpenGL.drawSmallText(gl, posLabel[2], (int) (xLeftWheel[0] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, posLabel[2]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 4), 0);
				OpenGL.drawSmallText(gl, wheelLabels[2], (int) (xLeftWheel[0] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, wheelLabels[2]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 5), 0);
				OpenGL.drawSmallText(gl, breakLabels[2], (int) (xLeftWheel[0] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, breakLabels[2]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 6), 0);
				
				OpenGL.drawSmallText(gl, posLabel[1], (int) (xLeftWheel[1] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, posLabel[1]) / 2)), (int) yReadingLabelBaseline, 0);
				OpenGL.drawSmallText(gl, wheelLabels[1], (int) (xLeftWheel[1] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, wheelLabels[1]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight), 0);
				OpenGL.drawSmallText(gl, breakLabels[1], (int) (xLeftWheel[1] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, breakLabels[1]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 2), 0);
				
				OpenGL.drawSmallText(gl, posLabel[3], (int) (xLeftWheel[1] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, posLabel[3]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 4), 0);
				OpenGL.drawSmallText(gl, wheelLabels[3], (int) (xLeftWheel[1] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, wheelLabels[3]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 5), 0);
				OpenGL.drawSmallText(gl, breakLabels[3], (int) (xLeftWheel[1] + (wheelWidth / 2) - (OpenGL.smallTextWidth(gl, breakLabels[3]) / 2)), (int) yReadingLabelBaseline - (OpenGL.smallTextHeight * 6), 0);
			}
			
		}
		
		return handler;
	}
	
	private float[] findTempColor(float min, float max, float current) { // Fourths then push this lol
		float tempHeight = max - min;
		float red = 255f * ((current - (min + (tempHeight / 3f))) / (tempHeight / 3f));
		float green = 255f * (1f - ((current - (min + ((tempHeight / 3f) * 2f))) / (tempHeight / 3f)));
		float blue = 255f * (1f - ((current - min) / (tempHeight / 3f)));
		
		if (blue < 0)
			blue = 0f;
		else if (blue > 255)
			blue = 255f;
		
		if (green < 0)
			green = 0f;
		else if (green > 255)
			green = 255f;
		
		if (red < 0)
			red = 0f;
		else if (red > 255)
			red = 255f;
		
		return new float[]{red / 255, green / 255, blue / 255, 1};
	}

	@Override
	public String toString() {
		return "Wheel Temps";
	}
}