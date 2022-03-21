import java.util.Map;

import com.jogamp.opengl.GL2ES3;

public class OpenGLDotPlot extends PositionedChart {

	Samples[] samples;
	
	int[][] bins; // [datasetN][binN]
	int binCount;
	
	// plot region
	float xPlotLeft;
	float xPlotRight;
	float plotWidth;
	float yPlotTop;
	float yPlotBottom;
	float plotHeight;
		
	// text label
	boolean showTextLabel;
	String textLabel;
	float yTextLabelBaseline;
	float yTextLabelTop;
	float xTextLabelLeft;
	float xTextLabelRight;
	
	// x-axis scale
	boolean showXaxisScale;
	Map<Float, String> xDivisions;
	float yXaxisTickTextBaseline;
	float yXaxisTickTextTop;
	float yXaxisTickBottom;
	float yXaxisTickTop;
	
	boolean xAxisIsCentered;
	float xCenterValue;
	boolean xAutoscaleMin;
	boolean xAutoscaleMax;
	float manualMinX;
	float manualMaxX;
	
	// control widgets
	WidgetDatasets datasetsWidget;
	WidgetCheckbox showTextLabelWidget;
	
	// constraints
	// constraints
		static final int SampleCountDefault = 1000;
		static final int SampleCountMinimum = 5;
		static final int SampleCountMaximum = Integer.MAX_VALUE;

		static final int BinCountDefault = 60;
		static final int BinCountMinimum = 2;
		static final int BinCountMaximum = Integer.MAX_VALUE;
		
		static final float xAxisMinimumDefault = -1;
		static final float xAxisMaximumDefault =  1;
		static final float xAxisCenterDefault  =  0;
		static final float xAxisLowerLimit     = -Float.MAX_VALUE;
		static final float xAxisUpperLimit     =  Float.MAX_VALUE;
		
		static final float yAxisRelativeFrequencyMinimumDefault = 0;
		static final float yAxisRelativeFrequencyMaximumDefault = 1;
		static final float yAxisRelativeFrequencyLowerLimit     = 0;
		static final float yAxisRelativeFrequencyUpperLimit     = 1;
		
		static final int yAxisFrequencyMinimumDefault = 0;
		static final int yAxisFrequencyMaximumDefault = 1000;
		static final int yAxisFrequencyLowerLimit     = 0;
		static final int yAxisFrequencyUpperLimit     = Integer.MAX_VALUE;
	
	AutoScale autoscalePower;
	
	public OpenGLDotPlot(int x1, int y1, int x2, int y2) {
		super(x1, y1, x2, y2);
		
		autoscalePower = new AutoScale(AutoScale.MODE_STICKY, 30, 0.20f);
		
		// Control Widgets
		datasetsWidget = new WidgetDatasets(4, new String[] {"X-Cord", "Y-Cord", "RPM", "Torque"}, newDatasets -> datasets = newDatasets);
		
		showTextLabelWidget = new WidgetCheckbox("Show Text Label", true, newShowTextLabel -> showTextLabel = newShowTextLabel);
		
		widgets = new Widget[2];
		
		widgets[0] = datasetsWidget;
		widgets[1] = showTextLabelWidget;
		
	}

	@Override
	public EventHandler drawChart(GL2ES3 gl, float[] chartMatrix, int width, int height, int lastSampleNumber, double zoomLevel, int mouseX, int mouseY) {
		
		EventHandler handler = null;

		float xCord = datasets.get(0).getSample(lastSampleNumber);
		float yCord = datasets.get(1).getSample(lastSampleNumber);
		float rpm = datasets.get(2).getSample(lastSampleNumber);
		float torque = datasets.get(3).getSample(lastSampleNumber);
		
		// Power (HP) = Torque (lb.in) x Speed (RPM) / 63,025
		float speed = torque * rpm / 63025;
		
		xPlotLeft = Theme.tilePadding;
		xPlotRight = width - Theme.tilePadding;
		plotWidth = xPlotRight - xPlotLeft;
		yPlotTop = height - Theme.tilePadding;
		yPlotBottom = Theme.tilePadding;
		plotHeight = yPlotTop - yPlotBottom;
		
		OpenGL.drawQuad2D(gl, Theme.plotBackgroundColor, xPlotLeft, yPlotBottom, xPlotRight, yPlotTop);
		
		return null;
	}

	@Override
	public String toString() {
		return "Dot Plot";
	}

}
