## Prepare documents
```
conda create -n tutorial
y
conda activate tutorial
pip install -r requirements.txt

make html-noplot
```

```
cd _build/html
python -m http.server  8080
```