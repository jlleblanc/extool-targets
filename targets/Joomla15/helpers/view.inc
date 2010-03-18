<?php
namespace Extool\Target;

class Joomla15View
{
	private $component;
	private $view;
	private $viewClass;
	private $viewDir;
	private $tables;
	private $tableNames;
	private $admin;
	
	private $templateType;
	private $dataType;
	private $listViews = array();

	private $snippets;
	private $rep;
	
	function __construct($view, $rep, $admin = false)
	{
		$this->view = $view;
		$this->admin = $admin;
		$this->rep = $rep;
		$this->component = $rep->name;

		$this->snippets = new \Extool\Helpers\SnippetFactory();
		$this->snippets->setBasePath('targets/Joomla15/snip');

		$this->viewDir = strtolower(str_replace(' ', '', $view->name));
		$this->viewClass = ucfirst($this->viewDir);
		
		if ($admin) {
			foreach ($rep->admin_views as $view) {
				if ($view->type == 'list') {
					$this->listViews[] = $view;
				}
			}
			$this->tables = $this->rep->admin_models[$view->name]->tables;
		} else {
			foreach ($rep->public_views as $view) {
				if ($view->type == 'list') {
					$this->listViews[] = $view;
				}
			}
			$this->tables = $this->rep->public_models[$view->name]->tables;
		}

		$this->tableNames = array_keys($this->tables);

		foreach ($this->listViews as &$view) {
			$view = str_replace('_', '', $view);
		}
	}
	
	public function makeViewClass()
	{
		if ($this->view->type == 'list') {
			return $this->makeMultipleViewClass();
		}
		
		return $this->makeSingleViewClass();
	}
	
	public function makeViewTmpl()
	{
		switch ($this->determineTemplateType()) {
			case 'adminlist':
				return $this->makeAdminListTmpl();
				break;
			
			case 'adminedit':
				return $this->makeAdminEditTmpl();
				break;
			
			case 'frontlist':
				return $this->makeFrontListTmpl();
				break;
			
			case 'frontsingle':
				return $this->makeFrontSingleTmpl();
				break;
		}
		
		
		return false;
	}
	
	private function makeAdminListTmpl()
	{
		$tmplSnip = $this->snippets->getSnippet('listadmin_tmpl');

		$first = true;

		foreach ($this->getTableFields() as $fieldTitle => $field) { 
			$header = $this->snippets->getSnippet('listadmin_tmpl_header');
			$rowCol = $this->snippets->getSnippet('listadmin_tmpl_rowcol');
			if ($first) {
				$header->assign('class', ' class="title"');
				$rowCol->assign('preWrap', '<a href="<?php echo $link; ?>">');
				$rowCol->assign('postWrap', '</a>');
				$first = false;
			} else {
				$header->assign('class', '');
				$rowCol->assign('preWrap', '');
				$rowCol->assign('postWrap', '');
			}
			
			$header->assign('fieldTitle', ucfirst($fieldTitle));
			$rowCol->assign('field', $field['name']);
			
			$tmplSnip->add('headers', $header);
			$tmplSnip->add('datarows', $rowCol);
		}
		
		
		$tmplSnip->assign('friendlyViewName', $this->view->name);
		$tmplSnip->assign('tablename', $this->tableNames[0]);
		$tmplSnip->assign('controller', $this->getDefaultController());
		$tmplSnip->assign('editviewname', preg_replace('/list$/', '', $this->viewDir));
		
		return $tmplSnip;
	}
	
	private function makeAdminEditTmpl()
	{		
		foreach ($this->getTableFields() as $fieldTitle => $field) {
			$colSnip = $this->snippets->getSnippet('editadmin_tmpl_row');
			$colSnip->assign('fieldTitle', ucfirst($fieldTitle));
			$colSnip->assign('fieldName', $field['name']);
			$fields[] = $colSnip;
		}
		
		$tmplSnip = $this->snippets->getSnippet('editadmin_tmpl');
		
		$tmplSnip->assign('friendlyViewName', $this->view->name);
		$tmplSnip->assign('controller', $this->getDefaultController());

		foreach ($fields as $field) {
			$tmplSnip->add('fields', $field);
		}
		
		return $tmplSnip;
	}
	
	private function makeFrontSingleTmpl()
	{
		$fields = array();
		
		$tmplSnip = $this->snippets->getSnippet('singlefront_tmpl');
		$tmplSnip->assign('viewname', str_replace(' ', '_', strtolower($this->view->name)));

		foreach ($this->getTableFields() as $fieldTitle => $fieldValue) {
			$field = $this->snippets->getSnippet('singlefront_tmpl_fields');
			$field->assign('fieldTitle', ucfirst($fieldTitle));
			$field->assign('fieldValue', $fieldValue['name']);
			$tmplSnip->add('fields', $field);
		}
		
		return $tmplSnip;
	}
	
	private function makeFrontListTmpl()
	{
		$tmplSnip = $this->snippets->getSnippet('listfront_tmpl');
		$tmplSnip->assign('viewname', str_replace(' ', '_', strtolower($this->view->name)));
		$tmplSnip->assign('tablename', $this->tableNames[0]);
		$tmplSnip->assign('singleviewname', preg_replace('/list$/i', '', $this->viewDir));

		$first = true;

		foreach ($this->getTableFields() as $fieldTitle => $fieldValue) {
			$header = $this->snippets->getSnippet('listfront_tmpl_header');
			$header->assign('header', $fieldTitle);
			$tmplSnip->add('headers', $header);

			$field = $this->snippets->getSnippet('listfront_tmpl_field');

			if ($first) {
				$field->assign('field', "<a href=\"<?php echo \$link; ?>\"><?php echo \$row->{$fieldValue} ?></a>");
				$first = false;
			} else {
				$field->assign('field', "<?php echo \$row->{$fieldValue} ?>");
			}

			$tmplSnip->add('fields', $field);
		}

		return $tmplSnip;
	}
	
	private function makeSingleViewClass()
	{
		$viewSnip = $this->snippets->getSnippet('view');
		$viewSnip->assign('component', ucfirst($this->component));
		$viewSnip->assign('view', $this->viewClass);
		
		$table = $this->tableNames[0];
		
		$rowCode = array();
		
		if ($this->determineTemplateType() == 'adminedit') {
			$dataSnip = $this->snippets->getSnippet('view_adminedit');
		} else {
			$dataSnip = $this->snippets->getSnippet('view_frontsingle');
		}
		
		if ($this->component != $table) {
			$table = $this->component . '_' . $table;
		} 
		
		$dataSnip->assign('table', $table);
		
		$viewSnip->assign('modelData', $dataSnip);
		
		$fileSnip = $this->snippets->getSnippet('code');
		$fileSnip->assign('code', $viewSnip);
		
		return $fileSnip;
	}
	
	private function makeMultipleViewClass()
	{
		$viewSnip = $this->snippets->getSnippet('view');
		$viewSnip->assign('component', ucfirst($this->component));
		$viewSnip->assign('view', $this->viewClass);

		foreach ($this->tableNames as $table) {
			$getModel = $this->snippets->getSnippet('view_getmodel');
			$getModel->assign('table', $table);
			$viewSnip->add('modelData', $getModel);
		}		
		
		$fileSnip = $this->snippets->getSnippet('code');
		$fileSnip->assign('code', $viewSnip);
		
		return $fileSnip;
	}
	
	private function determineTemplateType()
	{
		if (isset($this->templateType)) {
			return $this->templateType;
		}
		
		$list = false;
		
		if (preg_match('/list$/i', $this->viewDir)) {
			$list = true;
		}
		
		if ($this->admin) {
			if ($list) {
				$this->templateType = 'adminlist';
			} else {
				$this->templateType = 'adminedit';
			}
		} else {
			if ($list) {
				$this->templateType = 'frontlist';
			} else {
				$this->templateType = 'frontsingle';
			}
		}
		
		return $this->templateType;
	}
	
	// This helps us get the correct array of fields whether * or specific
	// fields are specified in the view
	private function getTableFields()
	{		
		$tableNames = array_keys($this->tables);
		$tableName = $tableNames[0];
		

		return $this->tables[$tableName]->fields;

		// $fields = array();
		// 
		// foreach ($this->tables[$tableName] as $field) {
		// 	$field = ucfirst($field);
		// 	$fields[$field] = strtolower(str_replace(' ', '_', $field));
		// }
	}
	
	private function getDefaultController()
	{
		$controller = $this->viewDir;
		
		if (!preg_match('/list$/i', $controller)) {
			if (in_array($controller . 'list', $this->listViews)) {	
				$controller .= 'list';
			}
		}
		
		return $controller;
	}
}