uniform bool isNegative;

void main(void)
{
    float col = isNegative? 1.0:0.0;
    //vec4 speed =	gl_FragData[1];
    gl_FragColor = vec4(col, col, col, 1.0);
}