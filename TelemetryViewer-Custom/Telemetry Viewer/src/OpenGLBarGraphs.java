import com.jogamp.opengl.GL2ES3;
import com.jogamp.opengl.GL3;

public class OpenGLBarGraphs extends PositionedChart {

	final int dialResolution = 400; // how many quads to draw
	final float dialThickness = 0.4f; // percentage of the radius
	Samples samples;
	boolean autoscaleMin;
	boolean autoscaleMax;
	float manualMin;
	float manualMax;

	// plot region
	float xPlotLeft;
	float xPlotRight;
	float plotWidth;
	float yPlotTop;
	float yPlotBottom;
	float plotHeight;

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
	String readingLabel;
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
	static final float BarMinimumDefault = 0;
	static final float BarMaximumDefault = 10;
	static final float BarLowerLimit = -Float.MAX_VALUE;
	static final float BarUpperLimit = Float.MAX_VALUE;

	static final int SampleCountDefault = 1000;
	static final int SampleCountLowerLimit = 1;
	static final int SampleCountUpperLimit = Integer.MAX_VALUE;

	// Data set amount bounds & variables
	int datasetAmount;
	int datasetAmountOriginal;
	final static int datasetAmountDefault = 1;
	final static int lowerDatasetAmount = 1;
	final static int upperDatasetAmount = 4;

	// Control Widgets
	WidgetDatasets datasetsWidget;
	WidgetTextfieldsOptionalMinMax minMaxWidget;
	WidgetTextfieldInteger sampleCountWidget;
	WidgetTextfieldInteger datasetAmountWidget;
	WidgetCheckbox showReadingLabelWidget;
	WidgetCheckbox showLegendWidget;
	WidgetCheckbox showMinMaxLabelsWidget;
	WidgetCheckbox showStatisticsWidget;

	public OpenGLBarGraphs(int x1, int y1, int x2, int y2) {
		super(x1, y1, x2, y2);

		datasetAmountWidget = new WidgetTextfieldInteger("Bar Graph Count", datasetAmountDefault, lowerDatasetAmount,
				upperDatasetAmount, newDatasetAmount -> datasetAmountOriginal = newDatasetAmount);

		datasetsWidget = new WidgetDatasets(4, new String[] { "Top Left", "Top Right", "Bottom Left", "Bottom Right" },
				newDatasets -> datasets = newDatasets);

		minMaxWidget = new WidgetTextfieldsOptionalMinMax("Bar", BarMinimumDefault, BarMaximumDefault, BarLowerLimit,
				BarUpperLimit, (newAutoscaleMin, newManualMin) -> {
					autoscaleMin = newAutoscaleMin;
					manualMin = newManualMin;
				}, (newAutoscaleMax, newManualMax) -> {
					autoscaleMax = newAutoscaleMax;
					manualMax = newManualMax;
				});

		showReadingLabelWidget = new WidgetCheckbox("Show Reading Label", true,
				newShowReadingLabel -> showReadingLabel = newShowReadingLabel);

		showMinMaxLabelsWidget = new WidgetCheckbox("Show Min/Max Labels", true,
				newShowMinMaxLabels -> showMinMaxLabels = newShowMinMaxLabels);

		showLegendWidget = new WidgetCheckbox("Show Legend", true, newShowLegend -> showLegend = newShowLegend);

		widgets = new Widget[6];

		widgets[0] = datasetAmountWidget;
		widgets[1] = datasetsWidget;
		widgets[2] = minMaxWidget;
		widgets[3] = showReadingLabelWidget;
		widgets[4] = showMinMaxLabelsWidget;
		widgets[5] = showLegendWidget;
	}

	@Override
	public EventHandler drawChart(GL2ES3 gl, float[] chartMatrix, int width, int height, int lastSampleNumber,
			double zoomLevel, int mouseX, int mouseY) {
		
		EventHandler handler = null;

		float[] barData = new float[datasetAmount];

		boolean haveDatasets = datasets != null && !datasets.isEmpty() && lastSampleNumber > -1;
		
		if (haveDatasets)
			datasetAmount = datasetAmountOriginal;
		else 
			datasetAmount = 0;
		
		for (int i = 0; i < datasetAmount; i++) {
			barData[i] = datasets.get(i).getSample(lastSampleNumber);
		}

		float[] barMin = new float[datasetAmount];
		float[] barMax = new float[datasetAmount];

		for (int i = 0; i < datasetAmount; i++) {
			barMin[i] = barData[i];
			barMax[i] = barData[i];
		}
		for (int i = 0; i < datasetAmount; i++) {
			for (int j = lastSampleNumber - 100; j < lastSampleNumber - 1; j++) {
				barMin[i] = datasets.get(i).getSample(j) < barMin[i] ? datasets.get(i).getSample(j) : barMin[i];
				barMax[i] = datasets.get(i).getSample(j) > barMax[i] ? datasets.get(i).getSample(j) : barMax[i];
			}
		}

		for (int i = 0; i < datasetAmount; i++) {
			barMin[i] = autoscaleMin ? barMin[i] : manualMin;
			barMax[i] = autoscaleMax ? barMax[i] : manualMax;
		}

		float[] xLeft = new float[datasetAmount];
		float[] yBottom = new float[datasetAmount];
		float barHeight = 10f;
		float barWidth = 50f;
		float padding;

		// calculate x and y positions of everything
		xPlotLeft = Theme.tilePadding;
		xPlotRight = width - Theme.tilePadding;
		plotWidth = xPlotRight - xPlotLeft;
		yPlotTop = height - Theme.tilePadding;
		yPlotBottom = Theme.tilePadding;
		plotHeight = yPlotTop - yPlotBottom;

		// Testing CUBE!
//		OpenGL.drawQuad2D(gl, new float[]{1f, 0f, 0f, 1f}, xPlotLeft, yPlotBottom, xPlotLeft + 40f, yPlotBottom + 40f);
		switch (datasetAmount) {
		case 1:
			barHeight = plotHeight * (8.5f / 10f);
			barWidth = plotWidth * (1.5f / 10f);
			xLeft[0] = xPlotLeft + ((plotWidth - barWidth) / 2);
			yBottom[0] = yPlotBottom + ((plotHeight - barHeight) / 2);
			break;
		case 2:
			barHeight = plotHeight * (8.5f / 10f);
			barWidth = plotWidth * (1.5f / 10f);
			yBottom[0] = yPlotBottom + ((plotHeight - barHeight) / 2);
			yBottom[1] = yPlotBottom + ((plotHeight - barHeight) / 2);
			padding = yBottom[0] - yPlotBottom;
			xLeft[0] = xPlotLeft + ((plotWidth / 2 - padding - barWidth));
			xLeft[1] = xPlotLeft + ((plotWidth / 2 + padding));
			break;
		case 3:
			barHeight = plotHeight * (8.5f / 10f);
			barWidth = plotWidth * (1.5f / 10f);
			yBottom[1] = yPlotBottom + ((plotHeight - barHeight) / 2);
			yBottom[2] = yPlotBottom + ((plotHeight - barHeight) / 2);
			barHeight *= (1.8f / 4);
			padding = yBottom[2] - yPlotBottom;
			yBottom[0] = yPlotTop - padding - barHeight;
			xLeft[0] = xPlotLeft + ((plotWidth - barWidth) / 2);
			xLeft[1] = xPlotLeft + ((plotWidth / 2 - padding - barWidth));
			xLeft[2] = xPlotLeft + ((plotWidth / 2 + padding));
			break;
		case 4:
			barHeight = plotHeight * (8.5f / 10f);
			barWidth = plotWidth * (1.5f / 10f);
			yBottom[2] = yPlotBottom + ((plotHeight - barHeight) / 2);
			yBottom[3] = yPlotBottom + ((plotHeight - barHeight) / 2);
			barHeight *= (1.8f / 4);
			padding = yBottom[2] - yPlotBottom;
			yBottom[0] = yPlotTop - padding - barHeight;
			yBottom[1] = yPlotTop - padding - barHeight;
			xLeft[0] = xPlotLeft + ((plotWidth / 2 - padding - barWidth));
			xLeft[1] = xPlotLeft + ((plotWidth / 2 + padding));
			xLeft[2] = xPlotLeft + ((plotWidth / 2 - padding - barWidth));
			xLeft[3] = xPlotLeft + ((plotWidth / 2 + padding));
			break;
		}

		for (int i = 0; i < datasetAmount; i++) {
			OpenGL.drawQuad2D(gl, Theme.plotBackgroundColor, xLeft[i], yBottom[i], xLeft[i] + barWidth,
					yBottom[i] + barHeight);
		}

		if (showMinMaxLabels) {
			for (int i = 0; i < datasetAmount; i++) {
				yMinMaxLabelsBaseline = Theme.tilePadding;
				yMinMaxLabelsTop = yMinMaxLabelsBaseline + OpenGL.smallTextHeight;
				minLabel = ChartUtils.formattedNumber(barMin[i], 3);
				maxLabel = ChartUtils.formattedNumber(barMax[i], 3);
				minLabelWidth = OpenGL.smallTextWidth(gl, minLabel);
				maxLabelWidth = OpenGL.smallTextWidth(gl, maxLabel);

				textHeight = OpenGL.smallTextHeight;

				int[] xValue = new int[2];
				int[] yValue = new int[] { (int) yBottom[i], (int) (yBottom[i] + barHeight - textHeight) };
				if (datasetAmount != 3) {
					if (i % 2 == 0)
						xValue = new int[] { (int) (xLeft[i] - minLabelWidth - 1f),
								(int) (xLeft[i] - maxLabelWidth - 1f) };
					else
						xValue = new int[] { (int) (xLeft[i] + barWidth + 2f), (int) (xLeft[i] + barWidth + 2f) };
				} else {
					if (i == 0 || i == 1)
						xValue = new int[] { (int) (xLeft[i] - minLabelWidth - 1f),
								(int) (xLeft[i] - maxLabelWidth - 1f) };
					else
						xValue = new int[] { (int) (xLeft[i] + barWidth + 2f), (int) (xLeft[i] + barWidth + 2f) };
				}

				OpenGL.drawSmallText(gl, minLabel, xValue[0], yValue[0], 0);
				OpenGL.drawSmallText(gl, maxLabel, xValue[1], yValue[1], 0);
			}
		}

		if (showReadingLabel) {
			textHeight = OpenGL.smallTextHeight;
			for (int i = 0; i < datasetAmount; i++) {
				dataTextWidth = OpenGL.smallTextWidth(gl, ChartUtils.formattedNumber(barData[i], 3));
				int xValue = (int) (xLeft[i] + ((barWidth - dataTextWidth) / 2f));
				int yValue = (int) (yBottom[i] - textHeight - 2f);

				OpenGL.drawSmallText(gl, ChartUtils.formattedNumber(barData[i], 3), xValue, yValue, 0);
			}
		}

		if (showLegend) {
			
		}
		
		/* 
		 * Define the area where the legend will be displayed
		 * Once defined create boundaries - boundaries can be ignored and manual choice of wether something is
		 * too large for the area or not is automatic
		 */
		
//		// Collect and set coordinates of legend
//		if (showLegend && haveDatasets) {
//			xLegendBorderLeft = Theme.tilePadding;
//			yLegendBorderBottom = Theme.tilePadding;
//			yLegendTextBaseline = yLegendBorderBottom + Theme.legendTextPadding;
//			yLegendTextTop = yLegendTextBaseline + OpenGL.mediumTextHeight;
//			yLegendBorderTop = yLegendTextTop + Theme.legendTextPadding;
//
//			legendMouseoverCoordinates = new float[datasetsCount][4];
//			legendBoxCoordinates = new float[datasetsCount][4];
//			xLegendNameLeft = new float[datasetsCount];
//
//			float xOffset = xLegendBorderLeft + (Theme.lineWidth / 2) + Theme.legendTextPadding;
//
//			for (int i = 0; i < datasetsCount; i++) {
//				legendMouseoverCoordinates[i][0] = xOffset - Theme.legendTextPadding;
//				legendMouseoverCoordinates[i][1] = yLegendBorderBottom;
//
//				legendBoxCoordinates[i][0] = xOffset;
//				legendBoxCoordinates[i][1] = yLegendTextBaseline;
//				legendBoxCoordinates[i][2] = xOffset + OpenGL.mediumTextHeight;
//				legendBoxCoordinates[i][3] = yLegendTextTop;
//
//				xOffset += OpenGL.mediumTextHeight + Theme.legendTextPadding;
//				xLegendNameLeft[i] = xOffset;
//				xOffset += OpenGL.mediumTextWidth(gl, datasets.get(i).name) + Theme.legendNamesPadding;
//
//				legendMouseoverCoordinates[i][2] = xOffset - Theme.legendNamesPadding + Theme.legendTextPadding;
//				legendMouseoverCoordinates[i][3] = yLegendBorderTop;
//			}
//
//			xLegendBorderRight = xOffset - Theme.legendNamesPadding + Theme.legendTextPadding + (Theme.lineWidth / 2);
//			if (showXaxisTitle)
//				xXaxisTitleTextLeft = xLegendBorderRight + ((xPlotRight - xLegendBorderRight) / 2)
//						- (OpenGL.largeTextWidth(gl, xAxisTitle) / 2.0f);
//
//			float temp = yLegendBorderTop + Theme.legendTextPadding;
//			if (yPlotBottom < temp) {
//				yPlotBottom = temp;
//				plotHeight = yPlotTop - yPlotBottom;
//			}
//		}

//		// draw the legend, if space is available
//		if (showLegend && haveDatasets && xLegendBorderRight < width - Theme.tilePadding) {
//			OpenGL.drawQuad2D(gl, Theme.legendBackgroundColor, xLegendBorderLeft, yLegendBorderBottom,
//					xLegendBorderRight, yLegendBorderTop);
//
//			for (int i = 0; i < datasetsCount; i++) {
//				Dataset d = datasets.get(i);
//				if (mouseX >= legendMouseoverCoordinates[i][0] && mouseX <= legendMouseoverCoordinates[i][2]
//						&& mouseY >= legendMouseoverCoordinates[i][1] && mouseY <= legendMouseoverCoordinates[i][3]) {
//					OpenGL.drawQuadOutline2D(gl, Theme.tickLinesColor, legendMouseoverCoordinates[i][0],
//							legendMouseoverCoordinates[i][1], legendMouseoverCoordinates[i][2],
//							legendMouseoverCoordinates[i][3]);
//					handler = EventHandler.onPress(event -> ConfigureView.instance.forDataset(d));
//				}
//				OpenGL.drawQuad2D(gl, d.glColor, legendBoxCoordinates[i][0], legendBoxCoordinates[i][1],
//						legendBoxCoordinates[i][2], legendBoxCoordinates[i][3]);
//				OpenGL.drawMediumText(gl, d.name, (int) xLegendNameLeft[i], (int) yLegendTextBaseline, 0);
//			}
//		}

		// clip to the plot region
		int[] originalScissorArgs = new int[4];
		gl.glGetIntegerv(GL3.GL_SCISSOR_BOX, originalScissorArgs, 0);
		gl.glScissor(originalScissorArgs[0] + (int) xPlotLeft, originalScissorArgs[1] + (int) yPlotBottom,
				(int) plotWidth, (int) plotHeight);

		// Draw insides of graphs
		if (haveDatasets) {
			for (int bar = 0; bar < datasetAmount; bar++) {
				float yValue = Math.max(((barData[bar] - barMin[bar]) / (barMax[bar] - barMin[bar]) * barHeight), 0);
				yValue = Math.min(yValue, (barMax[bar] - barMin[bar]) / (barMax[bar] - barMin[bar]) * barHeight);

				OpenGL.drawQuad2D(gl, datasets.get(bar).glColor, xLeft[bar], yBottom[bar], xLeft[bar] + barWidth,
						yValue + yBottom[bar]);
			}
		}

		for (int i = 0; i < datasetAmount; i++) {
			OpenGL.drawQuadOutline2D(gl, Theme.plotOutlineColor, xLeft[i], yBottom[i], xLeft[i] + barWidth,
					yBottom[i] + barHeight);
		}

		// stop clipping to the plot region
		gl.glScissor(originalScissorArgs[0], originalScissorArgs[1], originalScissorArgs[2], originalScissorArgs[3]);

		return handler;
	}

	@Override
	public String toString() {
		return "Bar Graphs";
	}
}
