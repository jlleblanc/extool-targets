<?php
/**
 * Extool Joomla 1.6 Target
 * 
 * Copyright 2010 Joseph LeBlanc
 * See LICENSE file for licensing details.
 * 
 */
namespace Extool\Target;

class Joomla16Controller
{
	private $component;
	private $admin;
	private $table;
	private $tablename;
	private $defaultView;

	private $rep;
	private $snippets;

	function __construct($rep, $defaultView, $admin = false, $table = '')
	{
		$this->admin = $admin;
		$this->table = strtolower(str_replace(' ', '_', $table));
		$this->tablename = $table;
		$this->defaultView = str_replace('_', '', $defaultView->system_name);

		$this->component = $rep->system_name;

		$this->snippets = new \Extool\Helpers\SnippetFactory();
		$this->snippets->setBasePath('targets/Joomla16/snip');
	}
	
	public function makeControllerCode()
	{
		$functions = $this->getFunctions();
		
		$controlSnip = $this->snippets->getSnippet('controller');
		$controlSnip->assign('component', ucfirst($this->component));
		$controlSnip->assign('functions', $functions);
		$controlSnip->assign('default_view', $this->defaultView);
		
		if ($this->admin) {
			$controlSnip->assign('controller_name', ucfirst($this->defaultView));
		} else {
			$controlSnip->assign('controller_name', '');
		}
		
		return $controlSnip;
	}
	
	private function getFunctions()
	{
		if (!$this->admin) {
			return '';
		}

		$funSnip = $this->snippets->getSnippet('controller_functions');
		$funSnip->assign('component', $this->component);

		if ($this->component == $this->table) {
			$table = $this->table;
		} else {
			$table = $this->component . '_' . $this->table;
		}

		$funSnip->assign('table', $table);
		$funSnip->assign('tablename', $this->tablename);
		$funSnip->assign('controller_name', $this->defaultView);

		// Yes, we're using the default view name here, as view and model names
		// should match.
		$funSnip->assign('modelName', $this->defaultView);

		$addView = preg_replace('/list$/i', '', $this->defaultView);
		$funSnip->assign('add_view', $addView);

		return $funSnip;
	}
}