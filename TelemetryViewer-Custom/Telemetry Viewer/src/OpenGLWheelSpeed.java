import java.nio.FloatBuffer;
import java.time.Duration;
import java.time.Instant;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.time.temporal.ChronoUnit;
import java.util.Date;
import java.util.Map;

import com.jogamp.opengl.GL2ES3;
import com.jogamp.opengl.GL3;

public class OpenGLWheelSpeed extends PositionedChart {

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

	// x-axis title
	boolean showXaxisTitle;
	float yXaxisTitleTextBasline;
	float yXaxisTitleTextTop;
	String xAxisTitle;
	float xXaxisTitleTextLeft;

	// legend
	boolean showLegend;
	float xLegendBorderLeft;
	float yLegendBorderBottom;
	float yLegendTextBaseline;
	float yLegendTextTop;
	float yLegendBorderTop;
	float[][] legendMouseoverCoordinates;
	float[][] legendBoxCoordinates;
	float[] xLegendNameLeft;
	float xLegendBorderRight;

	// x-axis scale
	boolean showXaxisScale;
	Map<Float, String> xDivisions;
	float yXaxisTickTextBaseline;
	float yXaxisTickTextTop;
	float yXaxisTickBottom;
	float yXaxisTickTop;

	// y-axis title
	boolean showYaxisTitle;
	float xYaxisTitleTextTop;
	float xYaxisTitleTextBaseline;
	String yAxisTitle;
	float yYaxisTitleText;

	// y-axis scale
	boolean showYaxisScale;
	Map<Float, String> yDivisions;
	float xYaxisTickTextBaseline;
	float xYaxisTickTextTop;
	float xYaxisTickBottom;
	float xYaxisTickTop;

	// constraints
	static final int SampleCountDefault = 1000;
	static final int SampleCountMinimum = 5;
	static final int SampleCountMaximum = Integer.MAX_VALUE;

	static final int BinCountDefault = 60;
	static final int BinCountMinimum = 2;
	static final int BinCountMaximum = Integer.MAX_VALUE;

	static final float xAxisMinimumDefault = -1;
	static final float xAxisMaximumDefault = 1;
	static final float xAxisCenterDefault = 0;
	static final float xAxisLowerLimit = -Float.MAX_VALUE;
	static final float xAxisUpperLimit = Float.MAX_VALUE;

	static final float yAxisRelativeFrequencyMinimumDefault = 0;
	static final float yAxisRelativeFrequencyMaximumDefault = 1;
	static final float yAxisRelativeFrequencyLowerLimit = 0;
	static final float yAxisRelativeFrequencyUpperLimit = 1;

	static final int yAxisFrequencyMinimumDefault = 0;
	static final int yAxisFrequencyMaximumDefault = 1000;
	static final int yAxisFrequencyLowerLimit = 0;
	static final int yAxisFrequencyUpperLimit = Integer.MAX_VALUE;

	// Max Speed
	static final int maxSpeedWidgetDefault = 200;
	int topSpeed;

	static final int lowerSpeedLimit = -1;
	static final int upperSpeedLimit = Integer.MAX_VALUE;

	static final int sampleCountDefault = 50;

	// control widgets
	WidgetDatasets datasetsWidget;
	WidgetTextfieldInteger sampleCountWidget;
	WidgetTextfieldInteger binCountWidget;
	WidgetTextfieldInteger topSpeedWidget;
	WidgetHistogramXaxisType xAxisTypeWidget;
	WidgetHistogramYaxisType yAxisTypeWidget;
	WidgetCheckbox showSprocketWidget;
	WidgetCheckbox showReadingLabelWidget;
	AutoScale autoscalePower;
	WidgetCheckbox showTextLabelWidget;
	boolean showTextLabel;
	
	boolean showSprocket;
	boolean showReadingLabel;

	// My points
	private float leftWheel;
	private float rightWheel;
	private float sprocket;
	
	// Time
	private Date lastFrame;
	
	private float lastSpotL = 0;
	private float lastSpotR = 0;
	private float lastSpotS = 0;
	
	public OpenGLWheelSpeed(int x1, int y1, int x2, int y2) {
		super(x1, y1, x2, y2);

		// Control Widgets
		datasetsWidget = new WidgetDatasets(3, new String[] { "Left", "Right", "Sprocket" },
				newDatasets -> datasets = newDatasets);

		topSpeedWidget = new WidgetTextfieldInteger("Top Speed", maxSpeedWidgetDefault, lowerSpeedLimit,
				upperSpeedLimit, newTopSpeed -> topSpeed = newTopSpeed);

		showSprocketWidget = new WidgetCheckbox("Show Sprocket", true,
				newShowSprocket -> showSprocket = newShowSprocket);

		showReadingLabelWidget = new WidgetCheckbox("Show Reading Labels", true,
				newShowReadingLabel -> showReadingLabel = newShowReadingLabel);
		
		topSpeedWidget = new WidgetTextfieldInteger("Speed", maxSpeedWidgetDefault, lowerSpeedLimit,
				upperSpeedLimit, newTopSpeed -> topSpeed = newTopSpeed);

		widgets = new Widget[5];
		
		widgets[0] = datasetsWidget;
		widgets[1] = topSpeedWidget;
		widgets[2] = showSprocketWidget;
		widgets[3] = showReadingLabelWidget;
		widgets[4] = topSpeedWidget;
	}
	
	@Override
	public EventHandler drawChart(GL2ES3 gl, float[] chartMatrix, int width, int height, int lastSampleNumber,
			double zoomLevel, int mouseX, int mouseY) {
		
		if (lastFrame == null)
			lastFrame = Date.from(Instant.now());
		
		EventHandler handler = null;
		
		// Just to make it not crash
		sampleCount = Math.min(sampleCount, lastSampleNumber);

		// calculate x and y positions of everything
		xPlotLeft = Theme.tilePadding;
		xPlotRight = width - Theme.tilePadding;
		plotWidth = xPlotRight - xPlotLeft;
		yPlotTop = height - Theme.tilePadding;
		yPlotBottom = Theme.tilePadding;
		plotHeight = yPlotTop - yPlotBottom;

		// get the samples
		int endIndex = lastSampleNumber;
		int startIndex = endIndex - (int) (sampleCount * zoomLevel) + 1;
		int minDomain = SampleCountMinimum - 1;
		if (endIndex - startIndex < minDomain)
			startIndex = endIndex - minDomain;
		if (startIndex < 0)
			startIndex = 0;
		sampleCount = endIndex - startIndex + 1;
		
		boolean haveDatasets = datasets != null && !datasets.isEmpty() && lastSampleNumber > -1;
		
		if (haveDatasets) {
			leftWheel = datasets.get(0).getSample(endIndex);
			rightWheel = datasets.get(1).getSample(endIndex);
			sprocket = datasets.get(2).getSample(endIndex);
		}
		
		// clip to the plot region
		int[] originalScissorArgs = new int[4];
		
		gl.glGetIntegerv(GL3.GL_SCISSOR_BOX, originalScissorArgs, 0);
		gl.glScissor(originalScissorArgs[0] + (int) xPlotLeft, originalScissorArgs[1] + (int) yPlotBottom,
				(int) plotWidth, (int) plotHeight);

		float xLeftWheelCord;
		float xRightWheelCord;
		float yWheelCord;
		float xSprocketCord;
		float ySprocketCord = 0;
		
		int wheelRadius =  (plotWidth) < (plotHeight) ? (int) plotWidth / 6 : (int) plotHeight / 5;
		
		
		// Wheel width	  = 15in
		// Sprocket width = 7in
		
		xSprocketCord = xPlotLeft + (plotWidth / 2);
		xLeftWheelCord = xPlotLeft + (plotWidth / 2) - (wheelRadius * 1.8f);
		xRightWheelCord = xPlotLeft + (plotWidth / 2) + (wheelRadius * 1.8f);
		
		if (showReadingLabel) {
			yWheelCord = yPlotTop - (plotHeight / 4);
			ySprocketCord = yPlotBottom + (plotHeight / 4);
		} else {
			yWheelCord = yPlotBottom + (plotHeight / 2);
		}

		Date currFrame = Date.from(Instant.now());
		long diff = Math.abs(currFrame.getTime() - lastFrame.getTime());
		float rpNano = 1173000000f;
		float leftMovement = (leftWheel / rpNano) * diff * -100000000;
		float rightMovement = (rightWheel / rpNano) * diff * -100000000;
		OpenGL.buffer.rewind();
		if (haveDatasets) {
			drawCircle(gl, new float[] {0, 0, 0, 1}, xLeftWheelCord, yWheelCord, wheelRadius);
			drawCircle(gl, new float[] {0, 0, 0, 1}, xRightWheelCord, yWheelCord, wheelRadius);
			
			drawCircle(gl, new float[] {1, 1, 1, 1}, xLeftWheelCord, yWheelCord, wheelRadius / 2);
			drawCircle(gl, new float[] {1, 1, 1, 1}, xRightWheelCord, yWheelCord, wheelRadius / 2);
			
			float xS = xSprocketCord;
			float yS = ySprocketCord;
			float degS;
			
			lastSpotL += (float) (leftMovement * Math.PI / 180.0f);
			lastSpotR += (float) (rightMovement * Math.PI / 180.0f);
			int spokeCount = 8;
			float theta = (float) ((Math.PI * 2)/spokeCount);
			drawInCircleLines(gl, xLeftWheelCord, yWheelCord, wheelRadius / 2, spokeCount, theta, lastSpotL);
			OpenGL.buffer.rewind();
			drawInCircleLines(gl, xRightWheelCord, yWheelCord, wheelRadius / 2, spokeCount, theta, lastSpotR);
			// Use pythagroeon theroum to find length of that piece
			if (showSprocket) {
				float spocketMovement = (sprocket / rpNano) * diff * 100000000;
				lastSpotS += Math.abs((float) (spocketMovement * Math.PI / 180.0f));
				
				OpenGL.buffer.rewind();
				int triangleCount = 64;
				theta = (float) ((Math.PI * 2)/triangleCount);
				float rightAngle = (float) (90f * Math.PI / 180f);
				for (int i = 1; i <= triangleCount; i++) {
					// Point outside of circle
					float length = (wheelRadius / 2) + (wheelRadius / 10);
					degS = (theta * i) + lastSpotS;
					
					// Point right of center
					xS = (float) ((length / 2) * Math.cos(Math.abs(rightAngle + degS))) + xSprocketCord; 	
					yS = (float) ((length / 2) * Math.sin(Math.abs(rightAngle + degS))) + yWheelCord;
					OpenGL.buffer.put(xS);		OpenGL.buffer.put(yS);
					
					// Outside point
					xS = (float) (length * Math.cos(degS)) + xSprocketCord; 	
					yS = (float) (length * Math.sin(degS)) + yWheelCord;
					OpenGL.buffer.put(xS);		OpenGL.buffer.put(yS);
					
					// Point left of center
					xS = (float) ((length / 2) * Math.cos(Math.abs(rightAngle - degS))) + xSprocketCord; 	
					yS = (float) ((length / 2) * Math.sin(Math.abs(rightAngle - degS))) + yWheelCord;
					OpenGL.buffer.put(xS);		OpenGL.buffer.put(yS);	
				}
				OpenGL.buffer.rewind();
				OpenGL.drawTrianglesXY(gl, GL3.GL_TRIANGLES, new float[] {0, 0, 0, 1}, OpenGL.buffer, triangleCount * 3);
				drawCircle(gl, new float[] {1, 1, 1, 1}, xSprocketCord, yWheelCord, wheelRadius / 2);
				OpenGL.buffer.rewind();
				theta = (float) ((Math.PI * 2)/16);
				drawInCircleLines(gl, xSprocketCord, yWheelCord, wheelRadius / 2, 16, theta, lastSpotS);
				drawCircle(gl, new float[] {0, 0, 0, 1}, xSprocketCord, yWheelCord, wheelRadius / 5); 
				drawCircle(gl, new float[] {1, 1, 1, 1}, xSprocketCord, yWheelCord, (wheelRadius / 5) - 2); 
			}
		}
		
		// stop clipping to the plot region
		gl.glScissor(originalScissorArgs[0], originalScissorArgs[1], originalScissorArgs[2], originalScissorArgs[3]);
		
		lastFrame = Date.from(Instant.now());
		
		return handler;
	}
	
	public static void drawInCircleLines(GL2ES3 gl, float x, float y, float length, int spokeCount, float theta, float lastSpot)
	{
		float lX;
		float lY;
		float deg = 0;
		for (int i = 1; i <= spokeCount; i++) {
			deg = (theta * i) + lastSpot;
			OpenGL.buffer.put(x); 	OpenGL.buffer.put(y);
			lX = (float) ((length) * Math.cos(deg)) + x;
			lY = (float) ((length) * Math.sin(deg)) + y;
			OpenGL.buffer.put(lX);	OpenGL.buffer.put(lY);
		}
		OpenGL.buffer.rewind();
		OpenGL.drawLinesXy(gl, GL3.GL_LINES, new float[] {0, 0, 0, 1}, OpenGL.buffer, spokeCount * 2);
	}
	
	public static void drawCircle(GL2ES3 gl, float[] color, float x, float y, int radius) 
	{
		double increment = 2 * Math.PI / 50;

		// Draw a bunch of triangles
		for (double angle = 0; angle < 2 * Math.PI; angle += increment) {
			float x1 = x + (float) Math.cos(angle) * radius;
			float y1 = y + (float) Math.sin(angle) * radius;
			float x2 = x + (float) Math.cos(angle + increment) * radius;
			float y2 = y + (float) Math.sin(angle + increment) * radius;

			OpenGL.drawTriangle2D(gl, color, x, y, x1, y1, x2, y2);
		}
	}

	@Override
	public String toString() {
		return "Wheel Speed";
	}
}
